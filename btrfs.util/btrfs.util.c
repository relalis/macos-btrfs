//
//  btrfs.c
//  macos-btrfs
//
//  Created by Yehia Hafez on 5/24/21.
//

#include <stddef.h>
#include <stdbool.h>

#include <sys/disk.h>
#include <sys/loadable_fs.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "btrfs_layout.h"
#include "btrfs_endian.h"

#ifndef FSUC_GETUUID
#define FSUC_GETUUID 'k'
#endif

static void usage(const char *progname) __attribute__((noreturn));
static void usage(const char *progname)
{
    fprintf(stderr, "usage: %s action_arg device_arg [mount_point_arg] [Flags]\n", progname);
    fprintf(stderr, "action_arg:\n");
    fprintf(stderr, "       -%c (Get UUID Key)\n", FSUC_GETUUID);
    fprintf(stderr, "       -%c (Mount)\n", FSUC_MOUNT);
    fprintf(stderr, "       -%c (Probe)\n", FSUC_PROBE);
    fprintf(stderr, "       -%c (Unmount)\n", FSUC_UNMOUNT);
    fprintf(stderr, "device_arg:\n");
    fprintf(stderr, "       device we are acting upon (for example, 'disk0s2')\n");
    fprintf(stderr, "mount_point_arg:\n");
    fprintf(stderr, "       required for Mount and Unmount\n");
    fprintf(stderr, "Flags:\n");
    fprintf(stderr, "       required for Mount and Probe\n");
    fprintf(stderr, "       indicates removable or fixed (for example 'fixed')\n");
    fprintf(stderr, "       indicates readonly or writable (for example 'readonly')\n");
    fprintf(stderr, "Flags (Mount only):\n");
    fprintf(stderr, "       indicates suid or nosuid (for example 'nosuid')\n");
    fprintf(stderr, "       indicates dev or nodev (for example 'nodev')\n");
    fprintf(stderr, "Examples:\n");
    fprintf(stderr, "       %s -p disk0s2 fixed writable\n", progname);
    fprintf(stderr, "       %s -m disk0s2 /my/hfs removable readonly nosuid nodev\n", progname);
    exit(FSUR_INVAL);
}

static int get_volume_superblock_record(char *rdev, BTRFS_SUPERBLOCK *sbrec) {
	
	BTRFS_SUPERBLOCK *sb;
	int f, opresult = FSUR_UNRECOGNIZED;
	void *buf;

	f = open(rdev, O_RDONLY);
	if (f == -1) return FSUR_IO_FAIL;

	buf = malloc(BTRFS_SUPERBLOCK_SIZE);
	if(!buf) {opresult = -ENOMEM;}
	else {
		// Read superblock record and check that the magic works
		if(pread(f, buf, BTRFS_SUPERBLOCK_SIZE, 0x10000) != BTRFS_SUPERBLOCK_SIZE) {opresult = FSUR_IO_FAIL;}
		else {
			sb = (BTRFS_SUPERBLOCK *)buf;
			if(!bcmp(sb->sb_magic, BTRFS_MAGIC, 0x8)) {
				memcpy(sbrec, sb, sizeof(*sb));
				opresult = FSUR_RECOGNIZED;
			}
			else {opresult = FSUR_UNRECOGNIZED;}
		}
	}

	if(buf) free(buf);
	(void) close(f);
	return opresult;
}

/**
 * do_getuuid - Get the UUID key of a BTRFS volume.
 *
 * If the volume is recognized as a BTRFS volume look up its UUID key if it
 * exists and output it to stdout then return FSUR_RECOGNIZED.
 *
 * If there is no UUID then return FSUR_INVAL and do not output anything to
 * stdout.
 *
 * If the volume is not an BTRFS volume return FSUR_UNRECOGNIZED.
 *
 * On error return FSUR_INVAL or FSUR_IO_FAIL.
 */

static int do_getuuid(char *rdev) {
	BTRFS_SUPERBLOCK *sb;
	int err = FSUR_INVAL;

	sb = malloc(BTRFS_SUPERBLOCK_SIZE);
	if(sb) {
		err = get_volume_superblock_record(rdev, sb);
		if(err == FSUR_RECOGNIZED) {
			char uuid[37];

			// todo: clean this up
			// right: 9d5bce63-3a73-4fb3-b8bd-9d00a6d9c30a
			// wrong: 63ce5b9d-733a-b34f-b8bd-9d00a6d9c30a
			if(snprintf(uuid, 37, "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
					 sb->fsid[0], sb->fsid[1], sb->fsid[2], sb->fsid[3],
					 sb->fsid[4], sb->fsid[5], sb->fsid[6], sb->fsid[7],
					 sb->fsid[8], sb->fsid[9],  sb->fsid[10], sb->fsid[11],
					 sb->fsid[12], sb->fsid[13], sb->fsid[14], sb->fsid[15]) != 36)
				err = FSUR_IO_FAIL;
			else {
				(void)write(STDOUT_FILENO, uuid, 36);
				err = FSUR_IO_SUCCESS;
			}
		}
		else fprintf(stderr, "Failure: %d\n", err);
		free(sb);
	} else err = -ENOMEM;

	return err;
}

/**
 * do_probe - Examine a volume to see if we recognize it as an NTFS volume.
 *
 * If the volume is recognized as an NTFS volume look up its volume label and
 * output it to stdout then return FSUR_RECOGNIZED.
 *
 * If the volume is not an NTFS volume return FSUR_UNRECOGNIZED.
 *
 * On error return FSUR_INVAL or FSUR_IO_FAIL.
 */
static int do_probe(char *rdev, const bool removable __attribute__((unused)),
		const bool writable __attribute__((unused)))
{
//	UUID="9d5bce63-3a73-4fb3-b8bd-9d00a6d9c30a" UUID_SUB="9cb965da-c2df-44ca-9917-47b96cc786a2" TYPE="btrfs" PARTUUID="ff67145c-01"
	int err = FSUR_IO_FAIL;
	BTRFS_SUPERBLOCK *sb;
	
	sb = malloc(BTRFS_SUPERBLOCK_SIZE);
	if(sb) {
		err = get_volume_superblock_record(rdev, sb);
		if(err == FSUR_RECOGNIZED) {
			(void)write(STDOUT_FILENO, sb->label, strlen(sb->label)+1);
		} else fprintf(stderr, "err: %d\n", err);
		free(sb);
	} else err = -ENOMEM;
	return err;
}


int main(int argc, char **argv) {
    char *progname, *dev, *mp = NULL;
    int err;
    char opt;
    bool removable, readonly, nosuid, nodev;
    char rawdev[MAXPATHLEN];
    char blockdev[MAXPATHLEN];
    struct stat sb;

    nodev = nosuid = readonly = removable = false;
    
    // Save & strip off program name.
    progname = argv[0];
    argc--;
    argv++;
    
    if (argc < 2 || argv[0][0] != '-')
        usage(progname);
    opt = argv[0][1];
    dev = argv[1];
    argc -= 2;
    argv += 2;

    // Check we have the right number of arguments
    switch (opt) {
		case FSUC_GETUUID:
            if (argc)
                usage(progname);
            break;
        case FSUC_PROBE:
            // For probe need the two mountflags also.
            if (argc != 2)
                usage(progname);
            break;
        case FSUC_MOUNT:
            // For mount need the mount point and four mountflags also.
            if (argc != 5)
                usage(progname);
            break;
        case FSUC_UNMOUNT:
            // For unmount need the mount point also.
            if (argc != 1)
                usage(progname);
            break;
        default:
            // Unsupported command.
            usage(progname);
            break;
    }
	
	err = snprintf(rawdev, sizeof(rawdev), "/dev/r%s", dev);
	if (err >= (int)sizeof(rawdev)) {
		fprintf(stderr, "%s: Specified device name is too long.\n",
				progname);
		exit(FSUR_INVAL);
	}
	if (stat(rawdev, &sb)) {
		fprintf(stderr, "%s: stat %s failed, %s\n", progname, rawdev,
				strerror(errno));
		exit(FSUR_INVAL);
	}
	err = snprintf(blockdev, sizeof(blockdev), "/dev/%s", dev);
	if (err >= (int)sizeof(blockdev)) {
		fprintf(stderr, "%s: Specified device name is too long.\n",
				progname);
		exit(FSUR_INVAL);
	}
	if (stat(blockdev, &sb)) {
		fprintf(stderr, "%s: stat %s failed, %s\n", progname, blockdev,
				strerror(errno));
		exit(FSUR_INVAL);
	}
	/* Get the mount point for the mount and unmount cases. */
	if (opt == FSUC_MOUNT || opt == FSUC_UNMOUNT) {
		mp = argv[0];
		argc--;
		argv++;
	}
	if (opt == FSUC_PROBE || opt == FSUC_MOUNT) {
		/* mountflag1: Removable or fixed. */
		if (!strcmp(argv[0], DEVICE_REMOVABLE))
			removable = true;
		else if (strcmp(argv[0], DEVICE_FIXED))
			usage(progname);
		/* mountflag2: Readonly or writable. */
		if (!strcmp(argv[1], DEVICE_READONLY))
			readonly = true;
		else if (strcmp(argv[1], DEVICE_WRITABLE))
			usage(progname);
		/* Only the mount command supports the third and fourth flag. */
		if (opt == FSUC_MOUNT) {
			/* mountflag3: Nosuid or suid. */
			if (!strcmp(argv[2], "nosuid"))
				nosuid = true;
			else if (strcmp(argv[2], "suid"))
				usage(progname);
			/* mountflag4: Nodev or dev. */
			if (!strcmp(argv[3], "nodev"))
				nodev = true;
			else if (strcmp(argv[3], "dev"))
				usage(progname);
		}
	}
	switch (opt) {
	case FSUC_GETUUID:
		err = do_getuuid(rawdev);
		break;
	case FSUC_PROBE:
		err = do_probe(rawdev, removable, readonly);
		break;
	case FSUC_MOUNT:
//		err = do_mount(progname, blockdev, mp, removable, readonly,
//				nosuid, nodev);
//		break;
	case FSUC_UNMOUNT:
//		err = do_unmount(progname, mp);
		break;
	default:
		/* Cannot happen... */
		usage(progname);
		break;
	}
	return err;
}
