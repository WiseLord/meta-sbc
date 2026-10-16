// SPDX-License-Identifier: GPL-2.0-only
/*
 * DRM driver for a Renesas R61581 480x320 SPI panel, 18-bit RGB666
 */

#include <linux/delay.h>
#include <linux/gpio/consumer.h>
#include <linux/module.h>
#include <linux/property.h>
#include <linux/slab.h>
#include <linux/spi/spi.h>

#include <drm/drm_atomic_helper.h>
#include <drm/clients/drm_client_setup.h>
#include <drm/drm_drv.h>
#include <drm/drm_fbdev_dma.h>
#include <drm/drm_fourcc.h>
#include <drm/drm_framebuffer.h>
#include <drm/drm_gem_atomic_helper.h>
#include <drm/drm_gem_dma_helper.h>
#include <drm/drm_gem_framebuffer_helper.h>
#include <drm/drm_mipi_dbi.h>
#include <drm/drm_modeset_helper.h>
#include <drm/drm_simple_kms_helper.h>
#include <video/mipi_display.h>

#define R61581_WIDTH		320
#define R61581_HEIGHT		480
#define R61581_BUF_SIZE		(R61581_WIDTH * R61581_HEIGHT * 3) // 460800 bytes

struct r61581_dev {
	struct mipi_dbi_dev dbidev;
	u8 *tx_buf;
};

static inline struct r61581_dev *to_r61581_dev(struct drm_device *drm)
{
	return container_of(drm, struct r61581_dev, dbidev.drm);
}

static const u32 r61581_formats[] = {
	DRM_FORMAT_XRGB8888,
};

static void r61581_enable(struct drm_simple_display_pipe *pipe,
			   struct drm_crtc_state *crtc_state,
			   struct drm_plane_state *plane_state)
{
	struct mipi_dbi_dev *dbidev = drm_to_mipi_dbi_dev(pipe->crtc.dev);
	struct mipi_dbi *dbi = &dbidev->dbi;
	int idx;

	if (!drm_dev_enter(pipe->crtc.dev, &idx))
		return;

	DRM_DEBUG_KMS("\n");

	mipi_dbi_hw_reset(dbi);
	msleep(150);

	mipi_dbi_command(dbi, MIPI_DCS_SOFT_RESET);
	msleep(100);

	mipi_dbi_command(dbi, MIPI_DCS_EXIT_SLEEP_MODE);
	msleep(200);

	mipi_dbi_command(dbi, 0xD0, 0x07, 0x42, 0x18);            /* Power setting */
	mipi_dbi_command(dbi, 0xD1, 0x00, 0x07, 0x10);            /* VCOM setting */
	mipi_dbi_command(dbi, 0xD2, 0x01, 0x02);                  /* Power, normal mode */
	mipi_dbi_command(dbi, 0xC0, 0x10, 0x3B, 0x00, 0x02, 0x11);/* Panel driving */
	mipi_dbi_command(dbi, 0xC5, 0x03);                        /* Frame rate 60 Hz */

	mipi_dbi_command(dbi, MIPI_DCS_SET_PIXEL_FORMAT, 0x06);   /* 18-bit RGB666 */
	mipi_dbi_command(dbi, MIPI_DCS_SET_ADDRESS_MODE, 0x48);   /* native portrait, BGR, MV=0 */

	mipi_dbi_command(dbi, 0xC8, 0x00, 0x32, 0x36, 0x45, 0x06, 0x16,
			  0x37, 0x75, 0x77, 0x54, 0x0C, 0x00);    /* Gamma */

	mipi_dbi_command(dbi, 0x20);                              /* Display inversion OFF */
	msleep(10);

	mipi_dbi_command(dbi, MIPI_DCS_SET_DISPLAY_ON);
	msleep(50);

	mipi_dbi_enable_flush(dbidev, crtc_state, plane_state);
	drm_dev_exit(idx);
}

static void r61581_pipe_update(struct drm_simple_display_pipe *pipe,
				struct drm_plane_state *old_state)
{
	struct mipi_dbi_dev *dbidev = drm_to_mipi_dbi_dev(pipe->crtc.dev);
	struct r61581_dev *rdev = to_r61581_dev(pipe->crtc.dev);
	struct mipi_dbi *dbi = &dbidev->dbi;
	struct drm_plane_state *state = pipe->plane.state;
	struct drm_framebuffer *fb = state ? state->fb : NULL;
	struct drm_gem_dma_object *dma_obj;
	const u8 *src;
	size_t stride, x, y, dst_idx = 0;
	int idx;

	if (!pipe->crtc.state->active || !fb || !rdev->tx_buf)
		return;

	if (!drm_dev_enter(pipe->crtc.dev, &idx))
		return;

	dma_obj = to_drm_gem_dma_obj(fb->obj[0]);
	src = dma_obj->vaddr;
	stride = fb->pitches[0];

	mipi_dbi_command(dbi, MIPI_DCS_SET_COLUMN_ADDRESS, 0x00, 0x00, 0x01, 0x3F); /* 0..319 */
	mipi_dbi_command(dbi, MIPI_DCS_SET_PAGE_ADDRESS,   0x00, 0x00, 0x01, 0xDF); /* 0..479 */
	mipi_dbi_command(dbi, MIPI_DCS_WRITE_MEMORY_START);

	gpiod_set_value_cansleep(dbi->dc, 1);

	for (y = 0; y < R61581_HEIGHT; y++) {
		const u8 *row = src + y * stride;

		for (x = 0; x < R61581_WIDTH; x++) {
			u8 b = row[x * 4 + 0];
			u8 g = row[x * 4 + 1];
			u8 r = row[x * 4 + 2];

			rdev->tx_buf[dst_idx++] = r & 0xFC;
			rdev->tx_buf[dst_idx++] = g & 0xFC;
			rdev->tx_buf[dst_idx++] = b & 0xFC;
		}
	}

	spi_write(dbi->spi, rdev->tx_buf, R61581_BUF_SIZE);

	drm_dev_exit(idx);
}

static void r61581_pipe_disable(struct drm_simple_display_pipe *pipe)
{
	struct mipi_dbi_dev *dbidev = drm_to_mipi_dbi_dev(pipe->crtc.dev);
	struct mipi_dbi *dbi = &dbidev->dbi;
	int idx;

	DRM_DEBUG_KMS("\n");

	if (!drm_dev_enter(pipe->crtc.dev, &idx))
		return;

	mipi_dbi_command(dbi, MIPI_DCS_SET_DISPLAY_OFF);

	drm_dev_exit(idx);
}

static const struct drm_simple_display_pipe_funcs r61581_pipe_funcs = {
	.enable     = r61581_enable,
	.disable    = r61581_pipe_disable,
	.update     = r61581_pipe_update,
};

static const struct drm_display_mode r61581_mode = {
	DRM_SIMPLE_MODE(R61581_WIDTH, R61581_HEIGHT, 49, 79),
};

DEFINE_DRM_GEM_DMA_FOPS(r61581_fops);

static struct drm_driver r61581_driver = {
	.driver_features = DRIVER_GEM | DRIVER_MODESET | DRIVER_ATOMIC,
	.fops            = &r61581_fops,
	DRM_GEM_DMA_DRIVER_OPS_VMAP,
	DRM_FBDEV_DMA_DRIVER_OPS,
	.debugfs_init    = mipi_dbi_debugfs_init,
	.name            = "r61581",
	.desc            = "Renesas R61581 Fast DMA",
	.major           = 1,
	.minor           = 0,
};

static const struct of_device_id r61581_of_match[] = {
	{ .compatible = "custom,r61581" },
	{ }
};
MODULE_DEVICE_TABLE(of, r61581_of_match);

static const struct spi_device_id r61581_id[] = {
	{ "r61581", 0 },
	{ }
};
MODULE_DEVICE_TABLE(spi, r61581_id);

static int r61581_probe(struct spi_device *spi)
{
	struct device *dev = &spi->dev;
	struct r61581_dev *rdev;
	struct mipi_dbi_dev *dbidev;
	struct drm_device *drm;
	struct mipi_dbi *dbi;
	struct gpio_desc *dc;
	int ret;

	rdev = devm_drm_dev_alloc(dev, &r61581_driver,
				     struct r61581_dev, dbidev.drm);
	if (IS_ERR(rdev))
		return PTR_ERR(rdev);

	dbidev = &rdev->dbidev;
	dbi = &dbidev->dbi;
	drm = &dbidev->drm;

	rdev->tx_buf = devm_kmalloc(dev, R61581_BUF_SIZE, GFP_KERNEL);
	if (!rdev->tx_buf)
		return -ENOMEM;

	dbi->reset = devm_gpiod_get(dev, "reset", GPIOD_OUT_HIGH);
	if (IS_ERR(dbi->reset))
		return dev_err_probe(dev, PTR_ERR(dbi->reset),
				     "Failed to get GPIO 'reset'\n");

	dc = devm_gpiod_get_optional(dev, "dc", GPIOD_OUT_LOW);
	if (IS_ERR(dc))
		return dev_err_probe(dev, PTR_ERR(dc),
				     "Failed to get GPIO 'dc'\n");

	ret = mipi_dbi_spi_init(spi, dbi, dc);
	if (ret)
		return ret;

	ret = mipi_dbi_dev_init_with_formats(dbidev, &r61581_pipe_funcs,
					      r61581_formats,
					      ARRAY_SIZE(r61581_formats),
					      &r61581_mode, 0, 0);
	if (ret)
		return ret;

	drm_mode_config_reset(drm);

	ret = drm_dev_register(drm, 0);
	if (ret)
		return ret;

	spi_set_drvdata(spi, drm);
	drm_client_setup(drm, NULL);

	return 0;
}

static void r61581_remove(struct spi_device *spi)
{
	struct drm_device *drm = spi_get_drvdata(spi);

	drm_dev_unplug(drm);
	drm_atomic_helper_shutdown(drm);
}

static void r61581_shutdown(struct spi_device *spi)
{
	drm_atomic_helper_shutdown(spi_get_drvdata(spi));
}

static struct spi_driver r61581_spi_driver = {
	.driver = {
		.name           = "r61581",
		.of_match_table = r61581_of_match,
	},
	.id_table = r61581_id,
	.probe    = r61581_probe,
	.remove   = r61581_remove,
	.shutdown = r61581_shutdown,
};
module_spi_driver(r61581_spi_driver);

MODULE_DESCRIPTION("High-Performance Renesas R61581 DRM driver (18-bit RGB666 SPI)");
MODULE_LICENSE("GPL");
