#include "pixmap.h"
#include <QtDebug>
#include <sstream>
#include <iomanip>
#include <algorithm>

Pixmap::Color::Color()
	: transparent(true), r(0), g(0), b(0)
{
}

Pixmap::Color::Color(uint8_t c_r, uint8_t c_g, uint8_t c_b)
	: transparent(false), r(c_r), g(c_g), b(c_b)
{
}

uint32_t Pixmap::Color::argb() const
{
	if(transparent) {
		return 0;
	}
	return (0xff << 24) | (r << 16) | (g << 8) | b;
}

Pixmap::Color Pixmap::Color::from_argb(uint32_t color)
{
	if((color >> 24) == 0) {
		return Color();
	}
	return Color((color >> 16) & 0xff, (color >> 8) & 0xff, color & 0xff);
}

Pixmap::Color Pixmap::Color::from_rgb(uint32_t color)
{
	return Color((color >> 16) & 0xff, (color >> 8) & 0xff, color & 0xff);
}

bool Pixmap::Color::operator==(const Color & other) const
{
	return (transparent == other.transparent) || (r == other.r && g == other.g && b == other.b);
}

		
Pixmap::Pixmap(const std::string & xpm_data)
	: w(1), h(1), pixels(1, 0), palette(1)
{
	load_from_xpm_data(xpm_data);
}

Pixmap::Pixmap(const std::vector<std::string> & xpm_lines)
	: w(1), h(1), pixels(1, 0), palette(1)
{
	load_from_xpm_lines(xpm_lines);
}

Pixmap::Pixmap(unsigned pixmap_width, unsigned pixmap_height, unsigned palette_size)
	: w(pixmap_width), h(pixmap_height), pixels(w * h, 0), palette(palette_size > 1 ? palette_size : 1, Color(0, 0, 0))
{
}

bool Pixmap::valid(unsigned x, unsigned y) const
{
	return x < w && y < h;
}

unsigned Pixmap::width() const
{
	return w;
}

unsigned Pixmap::height() const
{
	return h;
}

unsigned Pixmap::pixel(unsigned x, unsigned y) const
{
	if(valid(x, y)) {
		return pixels[x + y * w];
	}
	return 0;
}

unsigned Pixmap::color_count() const
{
	return palette.size();
}

Pixmap::Color Pixmap::color(unsigned index) const
{
	if(index < palette.size()) {
		return palette[index];
	}
	return Color();
}

bool Pixmap::resize(unsigned new_width, unsigned new_height)
{
	if(new_width < 1 || new_height < 1) {
		return false;
	}
	std::vector<unsigned> new_pixels(new_width * new_height, 0);
	std::vector<unsigned>::const_iterator src = pixels.begin();
	std::vector<unsigned>::iterator dest = new_pixels.begin();
	unsigned min_width = std::min(w, new_width);
	unsigned col = 0;
	unsigned rows = std::min(h, new_height);
	size_t src_inc = w > new_width ? w - new_width : 0;
	size_t dest_inc = w < new_width ? new_width - w : 0;
	while(rows) {
		*dest++ = *src++;
		++col;
		if(col == min_width) {
			if(--rows == 0) {
				break;
			}
			col = 0;
			src += src_inc;
			dest += dest_inc;
		}
	}
	pixels.swap(new_pixels);
	w = new_width;
	h = new_height;
	return true;
}

bool Pixmap::fill(unsigned index)
{
	if(index < palette.size()) {
		std::fill(pixels.begin(), pixels.end(), index);
		return true;
	}
	return false;
}

bool Pixmap::set_pixel(unsigned x, unsigned y, unsigned index)
{
	if(valid(x, y) && index < palette.size()) {
		pixels[x + y * w] = index;
		return true;
	}
	return false;
}

void run_floodfill(std::vector<unsigned> & pixels, unsigned width, unsigned height, unsigned x, unsigned y, unsigned what_index, unsigned to_what_index, int counter = 2e5)
{
	if(counter < 0) {
		return;
	}
	if(pixels[x + y * width] == to_what_index) {
		return;
	}
	if(pixels[x + y * width] != what_index) {
		return;
	}
	pixels[x + y * width] = to_what_index;
	if(x > 0) run_floodfill(pixels, width, height, x - 1, y, what_index, to_what_index, counter - 1);
	if(x < width - 1) run_floodfill(pixels, width, height, x + 1, y, what_index, to_what_index, counter - 1);
	if(y > 0) run_floodfill(pixels, width, height, x, y - 1, what_index, to_what_index, counter - 1);
	if(y < height - 1) run_floodfill(pixels, width, height, x, y + 1, what_index, to_what_index, counter - 1);
}

bool Pixmap::floodfill(unsigned x, unsigned y, unsigned index)
{
	if(valid(x, y) && index < palette.size()) {
		run_floodfill(pixels, w, h, x, y, pixel(x, y), index);
		return true;
	}
	return false;
}

unsigned Pixmap::add_color(Pixmap::Color new_color)
{
	palette.push_back(new_color);
	return palette.size() - 1;
}

bool Pixmap::set_color(unsigned index, Pixmap::Color new_color)
{
	if(index < palette.size()) {
		palette[index] = new_color;
		return true;
	}
	return false;
}

bool Pixmap::is_transparent_color(unsigned index) const
{
	if(index < palette.size()) {
		return palette[index].transparent;
	}
	return false;
}


void Pixmap::load_from_xpm_data(const std::string & xpm_data)
{
	std::vector<std::string> result;

	size_t interspace_start = 0;
	size_t first = xpm_data.find('"');
	size_t second = xpm_data.find('"', first + 1);
	while(first != std::string::npos && second != std::string::npos) {
		xpm_interspaces.push_back(xpm_data.substr(interspace_start, first - interspace_start));
		interspace_start = second + 1;

		result.push_back(xpm_data.substr(first + 1, second - first - 1));

		first = xpm_data.find('"', second + 1);
		second = xpm_data.find('"', first + 1);
	}
	xpm_interspaces.push_back(xpm_data.substr(interspace_start));

	load_from_xpm_lines(result);
}

void Pixmap::load_from_xpm_lines(const std::vector<std::string> & xpm_lines)
{
	if(xpm_lines.empty()) {
		throw Exception("Value line is missing");
	}
	std::vector<std::string>::const_iterator line = xpm_lines.begin();
	std::vector<std::string> values;
	xpm_values_interspaces.clear();
	xpm_values_interspaces.push_back("");
	bool last_was_space = true;
	for(unsigned i = 0; i < line->size(); ++i) {
		const char & ch = (*line)[i];
		if(ch == ' ') {
			if(!last_was_space) {
				xpm_values_interspaces.push_back("");
			}
			xpm_values_interspaces.back() += ch;
			last_was_space = true;
		} else {
			if(last_was_space) {
				values.push_back("");
			}
			values.back() += ch;
			last_was_space = false;
		}
	}
	++line;
	if(values.size() != 4) {
		throw Exception("Value line should be in format '<width> <height> <color_count> <char_per_pixel>'");
	}

	w = atoi(values[0].c_str());
	h = atoi(values[1].c_str());
	unsigned colors = atoi(values[2].c_str());
	unsigned cpp = atoi(values[3].c_str());
	if(w == 0 || h == 0 || colors == 0 || cpp == 0) {
		throw Exception("Values in value line should be integers and non-zero.");
	}

	xpm_color_count = 0;
	std::map<std::string, unsigned> color_names;
	palette.clear();
	while(colors --> 0) {
		if(line == xpm_lines.end()) {
			throw Exception("Color lines are missing or not enough");
		}

		if(line->size() <= cpp || (*line)[cpp] != ' ') {
			throw Exception("Color char should be followed by space in color table.");
		}
		std::string color_name = line->substr(0, cpp);

		std::vector<std::string> color_parts;
		std::vector<std::string> color_interspaces;
		color_interspaces.push_back("");
		bool last_was_space = true;
		for(unsigned i = cpp; i < line->size(); ++i) {
			const char & ch = (*line)[i];
			if(ch == ' ') {
				if(!last_was_space) {
					color_interspaces.push_back("");
				}
				color_interspaces.back() += ch;
				last_was_space = true;
			} else {
				if(last_was_space) {
					color_parts.push_back("");
				}
				color_parts.back() += ch;
				last_was_space = false;
			}
		}
		if(color_parts.size() < 1) {
			throw Exception("Color key is missing.");
		}
		if(color_parts.size() < 2) {
			throw Exception("Color value is missing.");
		}
		std::string key = color_parts[0];
		std::string value = color_parts[1];
		std::pair<std::string, std::pair<std::string, std::string> > result_pair;
		result_pair.first = color_interspaces[0];
		result_pair.second.first = color_interspaces[1];
		if(color_interspaces.size() > 2) {
			result_pair.second.second = color_interspaces[2];
		}
		xpm_colors_interspaces.push_back(result_pair);
		/*
		std::string color_name = line->substr(0, cpp);
		if(line->size() <= cpp || (*line)[cpp] != ' ') {
			throw Exception("Color char should be followed by space in color table.");
		}
		std::istringstream color_in(line->substr(cpp + 1));
		std::string key;
		std::string value;
		color_in >> key;
		if(!color_in) {
			throw Exception("Color key is missing.");
		}
		if(key != "c") {
			throw Exception("Only color key 'c' is supported.");
		}
		color_in >> value;
		if(!color_in) {
			throw Exception("Color value is missing.");
		}
		*/
		if(key != "c") {
			throw Exception("Only color key 'c' is supported.");
		}
		if(color_names.count(color_name) > 0) {
			throw Exception("Color <" + color_name + "> was found more than once.");
		}
		value = value.substr(1);
		bool is_zero = true;
		for(std::string::const_iterator it = value.begin(); it != value.end(); ++it) {
			if(*it != '0') {
				is_zero = false;
				break;
			}
		}
		unsigned color_value = strtoul(value.c_str(), 0, 16);
		if(color_value == 0 && !is_zero) {
			throw Exception("Color value is invalid.");
		}
		Color color = Color::from_rgb(color_value);
		color_names[color_name] = add_color(color);
		xpm_colors.push_back(color_name);
		++xpm_color_count;
		++line;
	}

	xpm_row_count = 0;
	pixels.clear();
	unsigned rows = h;
	while(rows --> 0) {
		if(line == xpm_lines.end()) {
			throw Exception("Pixel rows are missing or not enough");
		}
		if(line->size() % cpp != 0) {
			throw Exception("Pixel value in a row is broken.");
		} else if(line->size() < cpp * w) {
			throw Exception("Pixel row is too small.");
		} else if(line->size() > cpp * w) {
			throw Exception("Pixel row is too large.");
		}
		for(unsigned col = 0; col < w; ++col) {
			std::string pixel = line->substr(col * cpp, cpp);
			if(color_names.count(pixel) == 0) {
				throw Exception("Pixel value is invalid.");
			}
			pixels.push_back(color_names[pixel]);
		}
		++xpm_row_count;
		++line;
	}
	if(line != xpm_lines.end()) {
		throw Exception("Extra pixel rows are found.");
	}
}

std::string Pixmap::save() const
{
	std::string result;
	std::vector<std::string>::const_iterator interspace = xpm_interspaces.begin();
	if(interspace == xpm_interspaces.end()) {
		return result;
	}
	result += *interspace;

	std::ostringstream values;
	values << '"';
	values << xpm_values_interspaces[0] << w;
	values << xpm_values_interspaces[1] << h;
	values << xpm_values_interspaces[2] << color_count();
	values << xpm_values_interspaces[3] << 1;
	if(xpm_values_interspaces.size() >= 5) {
		values << xpm_values_interspaces[4];
	}
	values << '"';
	result += values.str();

	++interspace;
	if(interspace == xpm_interspaces.end()) {
		return result;
	}
	result += *interspace;

	std::vector<std::string>::const_iterator it_color_key = xpm_colors.begin();
	std::vector<std::pair<std::string, std::pair<std::string, std::string> > >::const_iterator it_color_interspace = xpm_colors_interspaces.begin();
	char free_color_key = 'a';
	unsigned current_color = 0;
	for(std::vector<Color>::const_iterator color = palette.begin(); color != palette.end(); ++color) {
		std::string color_key;
		if(it_color_key != xpm_colors.end()) {
			color_key = *it_color_key;
			++it_color_key;
		} else {
			color_key = free_color_key;
			++free_color_key;
		}

		std::pair<std::string, std::pair<std::string, std::string> > color_interspace;
		if(it_color_interspace != xpm_colors_interspaces.end()) {
			color_interspace = *it_color_interspace;
			++it_color_interspace;
		} else {
			color_interspace.first = " ";
			color_interspace.second.first = " ";
		}
		std::ostringstream color_value;
		color_value << std::hex << std::setw(6) << std::setfill('0') << (color->argb() & 0xffffff);
		result += std::string("\"") + color_key + color_interspace.first + "c" + color_interspace.second.first + "#" + color_value.str() + color_interspace.second.second + '"';

		bool print_original_interspace = (current_color < xpm_color_count - 1) || (current_color == color_count() - 1);
		if(print_original_interspace) {
			++interspace;
			if(interspace == xpm_interspaces.end()) {
				return result;
			}
			result += *interspace;
		} else {
			result += ",\n";
		}
		++current_color;
	}

	unsigned row_size = 0;
	unsigned current_row = 0;
	for(std::vector<unsigned>::const_iterator pixel = pixels.begin(); pixel != pixels.end(); ++pixel) {
		if(row_size == 0) {
			result += '"';
		}
		if(*pixel > xpm_colors.size()) {
			return result;
		}
		result += xpm_colors[*pixel];
		++row_size;
		if(row_size >= w) {
			result += '"';
			bool is_last_one = current_row == h - 1;
			bool print_original_interspace = (current_row < xpm_row_count - 1) || (current_row == h - 1);
			if(is_last_one) {
				std::string last_interspace;
				while(interspace != xpm_interspaces.end()) {
					last_interspace = *interspace++;
				}
				result += last_interspace;
			} else if(print_original_interspace) {
				++interspace;
				if(interspace == xpm_interspaces.end()) {
					return result;
				}
				result += *interspace;
			} else {
				result += ",\n";
			}

			row_size = 0;
			++current_row;
		}
	}
	return result;
}