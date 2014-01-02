//! Use `qmake "CONFIG += PIXMAP_TEST"` to build unit tests.
#include "pixmap.h"
#include <QtTest/QtTest>

class PixmapTest : public QObject {
	Q_OBJECT
private slots:
	void should_make_palette_with_one_opaque_black_color_by_default();
	void should_fill_image_with_default_color_on_create();
	void should_fill_image_with_color();
	void should_resize_image();
	void should_not_resize_image_if_width_is_zero();
	void should_not_resize_image_if_height_is_zero();
	void should_keep_existing_pixels_when_resizing_image();
	void should_fill_new_pixels_with_default_index_when_resizing_image();
	void should_get_pixel_index();
	void should_set_pixel_index();
	void should_flood_fill_area();
	void should_change_palette_color_value();
	void should_add_new_color_to_palette();
};

void PixmapTest::should_make_palette_with_one_opaque_black_color_by_default()
{
	Pixmap pixmap(2, 2);
	QCOMPARE(pixmap.color_count(), 1u);
	QCOMPARE(pixmap.color(0), 0xff000000);
}

void PixmapTest::should_fill_image_with_default_color_on_create()
{
	Pixmap pixmap(2, 2);
	for(unsigned x = 0; x < 2; ++x) {
		for(unsigned y = 0; y < 2; ++y) {
			QCOMPARE(pixmap.pixel(x, y), 0u);
		}
	}
}

void PixmapTest::should_fill_image_with_color()
{
	Pixmap pixmap(2, 2);
	unsigned color = pixmap.add_color(0xffffff);
	pixmap.fill(color);
	for(unsigned x = 0; x < 2; ++x) {
		for(unsigned y = 0; y < 2; ++y) {
			QCOMPARE(pixmap.pixel(x, y), color);
		}
	}
}

void PixmapTest::should_resize_image()
{
	Pixmap pixmap(2, 2);
	pixmap.resize(4, 4);
	QCOMPARE(pixmap.width(), 4u);
	QCOMPARE(pixmap.height(), 4u);
}

void PixmapTest::should_not_resize_image_if_width_is_zero()
{
	Pixmap pixmap(2, 2);
	pixmap.resize(0, 4);
	QCOMPARE(pixmap.width(), 2u);
	QCOMPARE(pixmap.height(), 2u);
}

void PixmapTest::should_not_resize_image_if_height_is_zero()
{
	Pixmap pixmap(2, 2);
	pixmap.resize(4, 0);
	QCOMPARE(pixmap.width(), 2u);
	QCOMPARE(pixmap.height(), 2u);
}

void PixmapTest::should_keep_existing_pixels_when_resizing_image()
{
	Pixmap pixmap(2, 2, 4);
	pixmap.set_pixel(0, 0, 0);
	pixmap.set_pixel(1, 0, 1);
	pixmap.set_pixel(0, 1, 2);
	pixmap.set_pixel(1, 1, 3);
	pixmap.resize(4, 4);
	QCOMPARE(pixmap.pixel(0, 0), 0u);
	QCOMPARE(pixmap.pixel(1, 0), 1u);
	QCOMPARE(pixmap.pixel(0, 1), 2u);
	QCOMPARE(pixmap.pixel(1, 1), 3u);
}

void PixmapTest::should_fill_new_pixels_with_default_index_when_resizing_image()
{
	Pixmap pixmap(2, 2, 2);
	pixmap.set_pixel(0, 0, 1);
	pixmap.set_pixel(1, 0, 1);
	pixmap.set_pixel(0, 1, 1);
	pixmap.set_pixel(1, 1, 1);
	pixmap.resize(2, 3);
	QCOMPARE(pixmap.pixel(0, 2), 0u);
	QCOMPARE(pixmap.pixel(1, 2), 0u);
}

void PixmapTest::should_get_pixel_index()
{
	Pixmap pixmap(2, 2, 2);
	pixmap.set_pixel(0, 0, 1);
	QCOMPARE(pixmap.pixel(0, 0), 1u);
}

void PixmapTest::should_set_pixel_index()
{
	Pixmap pixmap(2, 2, 2);
	pixmap.set_pixel(0, 0, 1);
	QCOMPARE(pixmap.pixel(0, 0), 1u);
}

void PixmapTest::should_flood_fill_area()
{
	Pixmap pixmap(8, 8, 3);
	unsigned data[64] = {
		0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 1, 1, 1, 0, 0, 0, 
		0, 1, 0, 0, 0, 1, 0, 0, 
		0, 0, 1, 0, 0, 1, 0, 0, 
		0, 0, 0, 1, 0, 1, 0, 0, 
		0, 0, 1, 0, 0, 0, 1, 0, 
		0, 1, 0, 0, 0, 0, 1, 0, 
		0, 1, 0, 0, 0, 0, 1, 0, 
	};
	for(unsigned i = 0; i < 64; ++i) {
		pixmap.set_pixel(i % 8, i / 8, data[i]);
	}
	pixmap.floodfill(5, 5, 2);
	unsigned result[64] = {
		0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 1, 1, 1, 0, 0, 0, 
		0, 1, 2, 2, 2, 1, 0, 0, 
		0, 0, 1, 2, 2, 1, 0, 0, 
		0, 0, 0, 1, 2, 1, 0, 0, 
		0, 0, 1, 2, 2, 2, 1, 0, 
		0, 1, 2, 2, 2, 2, 1, 0, 
		0, 1, 2, 2, 2, 2, 1, 0, 
	};
	for(unsigned i = 0; i < 64; ++i) {
		QCOMPARE(pixmap.pixel(i % 8, i / 8), result[i]);
	}
}

void PixmapTest::should_change_palette_color_value()
{
	Pixmap pixmap(2, 2, 2);
	pixmap.set_color(1, 0xff00ff00);
	QCOMPARE(pixmap.color(1), 0xff00ff00);
}

void PixmapTest::should_add_new_color_to_palette()
{
	Pixmap pixmap(2, 2, 2);
	unsigned index = pixmap.add_color(0xff00ff00);
	QCOMPARE(index, 2u);
	QCOMPARE(pixmap.color(2), 0xff00ff00);
}

QTEST_MAIN(PixmapTest)
#include "pixmap_test.moc"

