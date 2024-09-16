# libbtrfs/Makefile

# Compiler settings
CC		= clang
AR		= ar

# Directories
INCDIR		= include
SRCDIR		= src
BUILDDIR	= ../build
OBJDIR		= $(BUILDDIR)/obj

# Build vars
TARGET		= $(BUILDDIR)/btrfs.a
SRCS		= $(wildcard $(SRCDIR)/*.c)
OBJS		= $(SRCS:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

# Preprocessor flags
CFLAGS		= -I$(INCDIR) -Wall -Wextra -nostdlib -static -g -glldb # -Werror
ARFLAGS		= rcs

# Build targets
.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS) | $(BUILDDIR)
	$(AR) $(ARFLAGS) $@ $^

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR) $(OBJDIR):
	mkdir -p $@

clean:
	rm -rf $(OBJDIR) $(BUILDDIR)
