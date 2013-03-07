/**
 * @file
 *
 * @date Mar 6, 2013
 * @author: Anton Bondarev
 */
#include <errno.h>
#include <string.h>
#include <kernel/irq.h>

#include <drivers/tty.h>
#include <kernel/thread/sched.h>

size_t tty_read(struct tty *tty, char *buff, size_t size) {
	sched_lock();
		irq_lock();
		if (0 == tty->rx_queue.count) {
			event_wait_ms(&tty->rx_event, SCHED_TIMEOUT_INFINITE);
		}
		irq_unlock();

	sched_unlock();

	return 0;
}

size_t tty_write(struct tty *tty, char *buff, size_t size) {
	return 0;
}

int tty_ioctl(struct tty *tty, int request, void *data) {
	switch(request) {
	case TTY_IOCTL_GETATTR:
		memcpy(&tty->termios, tty, sizeof(struct termios));
		break;
	case TTY_IOCTL_SETATTR:
		memcpy(tty, &tty->termios, sizeof(struct termios));
		break;
	default:
		return -ENOSYS;
	}

	return ENOERR;
}
