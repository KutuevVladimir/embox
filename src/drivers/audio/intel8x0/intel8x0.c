/**
 * @file
 * @brief Driver for Intel Corporation 82801AA AC'97 Audio Controller (rev 01),
 * which is based on PortAudio library
 * @author Denis Deryugin <deryugin.denis@gmail.com>
 * @version 0.1
 * @date 2015-12-03
 */

#include <unistd.h>
#include <errno.h>
#include <util/log.h>
#include <stdio.h>
#include <asm/io.h>
#include <drivers/pci/pci.h>
#include <drivers/pci/pci_chip/pci_utils.h>
#include <drivers/pci/pci_driver.h>
#include <drivers/audio/portaudio.h>
#include <drivers/audio/audio_dev.h>
#include <drivers/audio/ac97.h>
#include <kernel/irq.h>

/* values for each busmaster block */

/* LVI */
#define ICH_REG_LVI_MASK		0x1f

/* SR */
#define ICH_FIFOE			0x10	/* FIFO error */
#define ICH_BCIS			0x08	/* buffer completion interrupt status */
#define ICH_LVBCI			0x04	/* last valid buffer completion interrupt */
#define ICH_CELV			0x02	/* current equals last valid */
#define ICH_DCH				0x01	/* DMA controller halted */

/* PIV */
#define ICH_REG_PIV_MASK		0x1f	/* mask */

/* CR */
#define ICH_IOCE			0x10	/* interrupt on completion enable */
#define ICH_FEIE			0x08	/* fifo error interrupt enable */
#define ICH_LVBIE			0x04	/* last valid buffer interrupt enable */
#define ICH_RESETREGS		0x02	/* reset busmaster registers */
#define ICH_STARTBM			0x01	/* start busmaster operation */


struct intel_ac_hw_dev {
	//uint32_t base_addr_nam;
	uint32_t base_addr_namb;
};

struct intel_ac_dev_priv {
	struct intel_ac_hw_dev *hw_dev;
	int devid;
	uint8_t *out_buf;
	uint32_t out_buf_len;
	uint8_t *in_buf;
	uint32_t in_buf_len;

	uint32_t cur_buff_offset;
};

static struct intel_ac_hw_dev intel_ac_hw_dev;

#define NAMB_REG(x) (intel_ac_hw_dev.base_addr_namb + (x))

/* You may want to look at "Intel 82801AA (ICH) & Intel 82801AB(ICH0)
 * IO Controller Hub AC ’97 Programmer’s Reference Manual" for more
 * technical details */
#define INTEL_AC_PCM_IN_BUF  0x00
#define INTEL_AC_PO_BUF 0x10
#define INTEL_AC_MIC_BUF     0x20

/* Last Valid Index */
#define INTEL_AC_PCM_IN_LVI  0x05
#define INTEL_AC_PO_LVI 0x15
#define INTEL_AC_MIC_LVI     0x25

/* Status registers */
#define INTEL_AC_PCM_IN_SR   0x06
#define INTEL_AC_PO_SR       0x16
#define INTEL_AC_MIC_SR      0x26

/* Control registers */
#define INTEL_AC_PCM_IN_CR  0x0B
#define INTEL_AC_PO_CR      0x1B
#define INTEL_AC_MIC_CR     0x2B

#define INTEL_AC_GLOB_CNT   0x2c
#define INTEL_AC_GLOB_STA   0x30
#define   ICH_TRI			0x20000000	/* ICH4: tertiary (AC_SDIN2) resume interrupt */
#define   ICH_TCR			0x10000000	/* ICH4: tertiary (AC_SDIN2) codec ready */
#define   ICH_BCS			0x08000000	/* ICH4: bit clock stopped */
#define   ICH_SPINT			0x04000000	/* ICH4: S/PDIF interrupt */
#define   ICH_P2INT			0x02000000	/* ICH4: PCM2-In interrupt */
#define   ICH_M2INT			0x01000000	/* ICH4: Mic2-In interrupt */
#define   ICH_SAMPLE_CAP	0x00c00000	/* ICH4: sample capability bits (RO) */
#define   ICH_SAMPLE_16_20	0x00400000	/* ICH4: 16- and 20-bit samples */
#define   ICH_MULTICHAN_CAP	0x00300000	/* ICH4: multi-channel capability bits (RO) */
#define   ICH_SIS_TRI		0x00080000	/* SIS: tertiary resume irq */
#define   ICH_SIS_TCR		0x00040000	/* SIS: tertiary codec ready */
#define   ICH_MD3			0x00020000	/* modem power down semaphore */
#define   ICH_AD3			0x00010000	/* audio power down semaphore */
#define   ICH_RCS			0x00008000	/* read completion status */
#define   ICH_BIT3			0x00004000	/* bit 3 slot 12 */
#define   ICH_BIT2			0x00002000	/* bit 2 slot 12 */
#define   ICH_BIT1			0x00001000	/* bit 1 slot 12 */
#define   ICH_SRI			0x00000800	/* secondary (AC_SDIN1) resume interrupt */
#define   ICH_PRI			0x00000400	/* primary (AC_SDIN0) resume interrupt */
#define   ICH_SCR			0x00000200	/* secondary (AC_SDIN1) codec ready */
#define   ICH_PCR			0x00000100	/* primary (AC_SDIN0) codec ready */
#define   ICH_MCINT			0x00000080	/* MIC capture interrupt */
#define   ICH_POINT			0x00000040	/* playback interrupt */
#define   ICH_PIINT			0x00000020	/* capture interrupt */
#define   ICH_NVSPINT		0x00000010	/* nforce spdif interrupt */
#define   ICH_MOINT			0x00000004	/* modem playback interrupt */
#define   ICH_MIINT			0x00000002	/* modem capture interrupt */
#define   ICH_GSCI			0x00000001	/* GPI status change interrupt */

#define INTEL_AC_CAS        0x34

#define INTEL_AC_SAMPLE_SZ 2  /* Bytes */
#define INTEL_AC_BUFFER_SZ 32 /* Buffer descriptors */

#define DESC_IOC (1 << 31)
#define DESC_BUP (1 << 30)

#define INTEL_AC_DESC_LEN    0xFF00
#define INTEL_AC_MAX_BUF_LEN (INTEL_AC_SAMPLE_SZ * INTEL_AC_BUFFER_SZ * INTEL_AC_DESC_LEN)

struct intel_ac_buff_desc {
	uint32_t pointer;           /* 2-byte aligned */
	uint32_t header;
	/* Header bits from high to low are
	 *     1 bit    Interrupt on completion
	 *     1 bit    Buffer underrun policy
	 *    14 bits   Reserved, must be zeroes
	 *    16 bits   Buffer length in samples
	 */
} __attribute__((aligned(2)));

/* This buffers could be allocated in runtime */
static struct intel_ac_buff_desc pcm_out_buff_list[INTEL_AC_BUFFER_SZ];
static struct intel_ac_buff_desc pcm_in_buff_list[INTEL_AC_BUFFER_SZ];
static uint8_t dac1_out_buf[INTEL_AC_MAX_BUF_LEN] __attribute__ ((aligned(0x1000)));
static uint8_t adc1_in_buf[INTEL_AC_MAX_BUF_LEN] __attribute__ ((aligned(0x1000)));

static uint8_t *_buf_by_dev(struct audio_dev *dev) {
	struct intel_ac_dev_priv *priv;
	priv = dev->ad_priv;
	
	switch(priv->devid) {
	case 0:
		return &dac1_out_buf[0];
	case 2:
		return &adc1_in_buf[0];
	default:
		return NULL;
	}
}

static struct intel_ac_buff_desc *_desc_list_by_dev(struct audio_dev *dev) {
	struct intel_ac_dev_priv *priv;
	priv = dev->ad_priv;
	
	switch(priv->devid) {
	case 0:
		return &pcm_out_buff_list[0];
	case 2:
		return &pcm_in_buff_list[0];
	default:
		return NULL;
	}
}

static int intel_ac_buf_init(int n, struct audio_dev *dev) {
	uint32_t buf = (uint32_t) _buf_by_dev(dev);
	struct intel_ac_buff_desc *desc_list = _desc_list_by_dev(dev);
	assert(buf);
	assert(desc_list);

	desc_list[n] = (struct intel_ac_buff_desc) {
		.pointer = buf + 2 * INTEL_AC_DESC_LEN * n,
		.header = INTEL_AC_DESC_LEN
	};

	return 0;
}

/* Intel Corporation 82801AA AC'97 Audio Controller,
 * provided by QEMU with flag -soundhw ac97 */
#define INTEL_AC_VID 0x8086
#define INTEL_AC_PID 0x2415

static irq_return_t iac_interrupt(unsigned int irq_num, void *dev_id) {
	uint8_t status;
	status = in8(NAMB_REG(INTEL_AC_PO_SR));
	log_debug("Status Register = %#x\n", status);

	if (!(status & 0xFF))
		return IRQ_NONE;

	out8(0x0, NAMB_REG(INTEL_AC_PO_CR));
	out8(0x0, NAMB_REG(INTEL_AC_MIC_CR));
	Pa_StartStream(NULL);

	out16(0x1C, NAMB_REG(INTEL_AC_PO_SR));
	out16(0x1C, NAMB_REG(INTEL_AC_PCM_IN_SR));
	out16(0x1C, NAMB_REG(INTEL_AC_MIC_SR));


	out32((1 << 15) | (1 << 11) | (1 << 10) | 1, NAMB_REG(INTEL_AC_GLOB_STA));

	return IRQ_HANDLED;
}

static int intel_ac_init(struct pci_slot_dev *pci_dev) {
	int err;
	assert(pci_dev);
	pci_set_master(pci_dev);

	intel_ac_hw_dev.base_addr_namb = pci_dev->bar[1] & 0xFF00;

	if ((err = ac97_init(pci_dev)))
		return err;

	if ((err = irq_attach(pci_dev->irq, iac_interrupt, IF_SHARESUP, &intel_ac_hw_dev, "iac")))
		return err;

	return 0;
}

PCI_DRIVER("Intel Corporation 82801AA AC'97 Audio Controller", intel_ac_init, INTEL_AC_VID, INTEL_AC_PID);


static void intel_ac_dev_start(struct audio_dev *dev) {
	uint8_t tmp;
	int i;
	uint8_t buf;
	uint8_t lvi;
	uint8_t cr;
	switch (((struct intel_ac_dev_priv*)dev->ad_priv)->devid) {
	case 0:
		buf = INTEL_AC_PO_BUF;
		lvi = INTEL_AC_PO_LVI;
		cr  = INTEL_AC_PO_CR;
		break;
	case 2:
		buf = INTEL_AC_MIC_BUF;
		lvi = INTEL_AC_MIC_LVI;
		cr  = INTEL_AC_MIC_CR;
		break;
	default:
		log_error("Unsupported AC97 device id!");
		return;
	}
	out32((uint32_t)_desc_list_by_dev(dev), NAMB_REG(buf));

	/* Setup buffers, currently just zeroes */
	for (i = 0; i < INTEL_AC_BUFFER_SZ; i++) {
		intel_ac_buf_init(i, dev);
	}

	/* Setup Last Valid Index */
	out8(INTEL_AC_BUFFER_SZ - 1, NAMB_REG(lvi));

	/* Set run bit */
	tmp = ICH_IOCE | ICH_STARTBM | ICH_FEIE | ICH_LVBIE;
	out8(tmp, NAMB_REG(cr));
}

static void intel_ac_dev_pause(struct audio_dev *dev) {
	uint8_t cr;
	switch (((struct intel_ac_dev_priv*)dev->ad_priv)->devid) {
	case 0:
		cr  = INTEL_AC_PO_CR;
		break;
	case 2:
		cr  = INTEL_AC_MIC_CR;
		break;
	default:
		log_error("Unsupported AC97 device id!");
		return;
	}

	out8(0x0, NAMB_REG(cr));
}

static void intel_ac_dev_resume(struct audio_dev *dev) {
	intel_ac_dev_start(dev);
}

static void intel_ac_dev_stop(struct audio_dev *dev) {
	uint8_t cr;
	switch (((struct intel_ac_dev_priv*)dev->ad_priv)->devid) {
	case 0:
		cr  = INTEL_AC_PO_CR;
		break;
	case 2:
		cr  = INTEL_AC_MIC_CR;
		break;
	default:
		log_error("Unsupported AC97 device id!");
		return;
	}

	out8(0x0, NAMB_REG(cr));
}

static int intel_ac_ioctl(struct audio_dev *dev, int cmd, void *args) {
	int devid = ((struct intel_ac_dev_priv*)dev->ad_priv)->devid;
	switch(cmd) {
	case ADIOCTL_IN_SUPPORT:
		return devid == 2 ?
			AD_MONO_SUPPORT | AD_16BIT_SUPPORT : 0;
	case ADIOCTL_OUT_SUPPORT:
		return devid == 0 ?
			AD_STEREO_SUPPORT | AD_16BIT_SUPPORT : 0;
	case ADIOCTL_BUFLEN:
		return INTEL_AC_MAX_BUF_LEN;
	}
	SET_ERRNO(EINVAL);
	return -1;
}

static const struct audio_dev_ops intel_ac_dev_ops = {
	.ad_ops_start  = intel_ac_dev_start,
	.ad_ops_pause  = intel_ac_dev_pause,
	.ad_ops_resume = intel_ac_dev_resume,
	.ad_ops_stop   = intel_ac_dev_stop,
	.ad_ops_ioctl  = intel_ac_ioctl
};

static struct intel_ac_dev_priv intel_ac_dac1 = {
	.hw_dev      = &intel_ac_hw_dev,
	.devid       = 0,
	.out_buf     = dac1_out_buf,
	.out_buf_len = sizeof(dac1_out_buf),
};

static struct intel_ac_dev_priv intel_ac_dac2 = {
	.hw_dev = &intel_ac_hw_dev,
	.devid  = 1
};

static struct intel_ac_dev_priv intel_ac_adc1 = {
	.hw_dev      = &intel_ac_hw_dev,
	.devid       = 2,
	.in_buf      = adc1_in_buf,
	.in_buf_len  = sizeof(adc1_in_buf)
};

AUDIO_DEV_DEF("intel_ac_dac1", (struct audio_dev_ops *)&intel_ac_dev_ops, &intel_ac_dac1);
AUDIO_DEV_DEF("intel_ac_dac2", (struct audio_dev_ops *)&intel_ac_dev_ops, &intel_ac_dac2);
AUDIO_DEV_DEF("intel_ac_adc1", (struct audio_dev_ops *)&intel_ac_dev_ops, &intel_ac_adc1);

uint8_t *audio_dev_get_out_cur_ptr(struct audio_dev *audio_dev) {
	return _buf_by_dev(audio_dev);
}
