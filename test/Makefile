# btrfs/Makefile

# Compiler settings
CC			= clang
LD			= ld

# Directories
INCDIR		= ../kernel/include
SRCDIR		= .
BUILDDIR	= ../build
OBJDIR		= $(BUILDDIR)/obj

# Build vars
TARGET		= $(BUILDDIR)/btrfs_test
SRCS		= $(wildcard $(SRCDIR)/*.c)
OBJS		= $(SRCS:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

# Preprocessor flags
CFLAGS		= -I$(INCDIR) -Wall -Wextra -g -glldb # -Werror
LDFLAGS		= 

# Build targets
.PHONY: all clean

all: $(TARGET)

$(BUILDDIR) $(OBJDIR):
	mkdir -p $@

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJS) | $(BUILDDIR)
	$(CC) $(OBJS) $(CFLAGS) -o $@ $(LDFLAGS)


clean:
	rm -rf $(OBJDIR) $(BUILDDIR)