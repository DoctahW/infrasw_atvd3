CC      = gcc
TARGET  = scheduler
LOGIN   = jems2
SRCDIR  = source
OBJDIR  = objects

CFLAGS  = -std=c11 -Wall -Wextra -O2 -D_POSIX_C_SOURCE=200809L -DLOGIN='"$(LOGIN)"'
LDFLAGS =
LDLIBS  =

SRCS = $(wildcard $(SRCDIR)/*.c)
OBJS = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRCS))
DEPS = $(OBJS:.o=.d)

OUTS = rate_$(LOGIN).out edf_$(LOGIN).out

.PHONY: all clean distclean rebuild

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

rebuild: clean all

clean:
	rm -rf $(OBJDIR) $(TARGET)

distclean: clean
	rm -f $(OUTS)

-include $(DEPS)
