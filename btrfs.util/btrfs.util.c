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
#include <sys/mount.h>

#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "btrfs.h"
#include "crc32c.h"

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

static int get_volume_superblock_record(char *rdev, superblock *sbrec) {

	superblock *sb;
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
			sb = (superblock *)buf;
			if(sb->magic == BTRFS_MAGIC) {
				uint32_t crc32 = ~calc_crc32c(0xffffffff, (uint8_t*)&sb->uuid, (unsigned long)sizeof(superblock) - sizeof(sb->checksum));
				if (crc32 == *((uint32_t*)sb->checksum)) {
					memcpy(sbrec, sb, sizeof(*sb));
					opresult = FSUR_RECOGNIZED;
				}
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
	superblock *sb;
	int err = FSUR_INVAL;

	sb = malloc(BTRFS_SUPERBLOCK_SIZE);
	if(sb) {
		err = get_volume_superblock_record(rdev, sb);
		if(err == FSUR_RECOGNIZED) {
			char uuid[37];

			/// @todo: clean this up
			if(snprintf(uuid, 37, "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
					 sb->uuid.uuid[0], sb->uuid.uuid[1], sb->uuid.uuid[2], sb->uuid.uuid[3],
					 sb->uuid.uuid[4], sb->uuid.uuid[5], sb->uuid.uuid[6], sb->uuid.uuid[7],
					 sb->uuid.uuid[8], sb->uuid.uuid[9],  sb->uuid.uuid[10], sb->uuid.uuid[11],
					 sb->uuid.uuid[12], sb->uuid.uuid[13], sb->uuid.uuid[14], sb->uuid.uuid[15]) != 36)
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
 * If the volume is recognized as an dfghsa volume look up its volume label and
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
	superblock *sb;
	
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

/**
 * do_exec - Execute an external command.
 */
static int do_exec(const char *progname, char *const args[])
{
	pid_t pid;
	union wait status;
	int err;

	pid = fork();
	if (pid == -1) {
		fprintf(stderr, "%s: fork failed: %s\n", progname,
				strerror(errno));
		return FSUR_INVAL;
	}
	if (!pid) {
		/* In child process, execute external command. */
		(void)execv(args[0], args);
		/* We only get here if the execv() failed. */
		err = errno;
		fprintf(stderr, "%s: execv %s failed: %s\n", progname, args[0],
				strerror(err));
		exit(err);
	}
	/* In parent process, wait for exernal command to finish. */
	if (wait4(pid, (int*)&status, 0, NULL) != pid) {
		fprintf(stderr, "%s: BUG executing %s command.\n", progname,
				args[0]);
		return FSUR_INVAL;
	}
	if (!WIFEXITED(status)) {
		fprintf(stderr, "%s: %s command aborted by signal %d.\n",
				progname, args[0], WTERMSIG(status));
		return FSUR_INVAL;
	}
	err = WEXITSTATUS(status);
	if (err) {
		fprintf(stderr, "%s: %s command failed: %s\n", progname,
				args[0], strerror(err));
		return FSUR_IO_FAIL;
	}
	return FSUR_IO_SUCCESS;
}

/**
 * do_mount - Mount a file system.
 */
static int do_mount(const char *progname, char *dev, char *mp,
		const bool removable __attribute__((unused)),
		const bool readonly, const bool nosuid, const bool nodev)
{
	char *const kextargs[] = { "/sbin/kextload",
			"/System/Library/Extensions/btrfs.kext", NULL };
	char *mountargs[] = { "/sbin/mount", "-w", "-o",
			"suid", "-o", "dev", "-t", "btrfs", dev, mp, NULL };
	struct vfsconf vfc;

	if (!mp || !strlen(mp))
		return FSUR_INVAL;
	if (readonly)
		mountargs[1] = "-r";
	if (nosuid)
		mountargs[3] = "nosuid";
	if (nodev)
		mountargs[5] = "nodev";
	/*
	 * If the kext is not loaded, load it now.  Ignore any errors as the
	 * mount will fail appropriately if the kext is not loaded.
	 */
	if (getvfsbyname("btrfs", &vfc))
		(void)do_exec(progname, kextargs);
	return do_exec(progname, mountargs);
}

/**
 * do_unmount - Unmount a volume.
 */
static int do_unmount(const char *progname, char *mp)
{
	char *const umountargs[] = { "/sbin/umount", mp, NULL };

	if (!mp || !strlen(mp))
		return FSUR_INVAL;
	return do_exec(progname, umountargs);
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
		err = do_mount(progname, blockdev, mp, removable, readonly,
				nosuid, nodev);
		break;
	case FSUC_UNMOUNT:
		err = do_unmount(progname, mp);
		break;
	default:
		usage(progname);
		break;
	}
	return err;
}
