#pragma once

#include <cstdint>
#include <cstddef>
#include <cctype>
#include <cstdio>
#include <array>
#include <memory>
#include <string>
#include <map>
#include <tuple>
#include <type_traits>
#include <system_error>
#include <algorithm>
#include <bcm2835.h>

using std::array;
using std::unique_ptr;
using std::make_unique;
using std::map;
using std::string;
using std::add_pointer_t;
using std::tuple;

using HRESET = int;
using address = uint8_t;

constexpr HRESET F_INIT = -1;
constexpr HRESET SUCCESS = 0;


//struct write_pack
//{
//	address addr;
//	uint8_t	data;
//};

using write_pack = tuple<address, uint8_t>;

enum class Pins
{
	CS = RPI_GPIO_P1_24
};

template<typename T = size_t>
static inline constexpr T Pin(Pins val)
{
	return static_cast<T>(val);
}


template<size_t w, size_t h>
class matrix
{
public:
	enum class PART_TYPE
	{
		LEFT,
		RIGHT
	};

public:
	using raw_data_type = array<array<bool, w>, h>;
	using data_type = array<int, h>;
	using size_type = size_t;

private:
	raw_data_type data = { };

public:
	matrix();

	matrix(array<int, h> d);

	void set_data(size_type line, size_type column, bool val);

	data_type get_data()const;

	raw_data_type get_raw_data()const;

	data_type part_get(size_type column, PART_TYPE type)const;

	static data_type combine(data_type left, data_type right);


};

template<size_t w, size_t h>
inline matrix<w, h>::matrix()
{
	for (auto &l : data)
	{
		l.fill(false);
	}
}

template<size_t w, size_t h>
inline matrix<w, h>::matrix(array<int, h> d)
{
	int ht = 0;
	for (auto l : d)
	{
		for (int i = 0; i < h; i++)
		{
			data[ht][w - i - 1] = (l >> i) & 1;
		}
		ht++;
	}
}

template<size_t w, size_t h>
inline void matrix<w, h>::set_data(size_type line, size_type column, bool val)
{
	data[line][column] = val;
}

template<size_t w, size_t h>
inline typename matrix<w, h>::data_type matrix<w, h>::get_data()const
{
	auto b2i = [](bool val)
	{
		return static_cast<int>(val);
	};

	data_type ret = {};

	for (int i = 0; i < h; i++)
	{
		int line = 0;
		for (int j = 0; j < w; j++)
		{
			line |= (static_cast<int>((data[i][j] == true ? 0x01 : 0x00)) << (w - j - 1));
		}
		ret[i] = line;
	}

	return ret;
}

template<size_t w, size_t h>
inline typename matrix<w, h>::raw_data_type matrix<w, h>::get_raw_data()const
{
	return this->data;
}

template<size_t w, size_t h>
inline typename matrix<w, h>::data_type matrix<w, h>::part_get(size_type column, PART_TYPE type)const
{
	data_type ret = this->get_data();
	if (type == PART_TYPE::LEFT)
	{
		for (auto &l : ret)
		{
			l >>= (w - column - 1);
		}
	}
	else if (type == PART_TYPE::RIGHT)
	{
		for (auto &l : ret)
		{
			l <<= column;
		}
	}

	return ret;
}

template<size_t w, size_t h>
inline typename matrix<w, h>::data_type matrix<w, h>::combine(data_type a, data_type b)
{
	data_type ret = {};
	for (int i = 0; i < h; i++)
	{
		ret[i] = a[i] | b[i];
	}
	return ret;
}



template<size_t w, size_t h >
class font_lib
{
public:
	using data_type = matrix<w, h>;

private:

	const char characters[37] =
	{
		'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
		'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
		'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
		'U', 'V', 'W', 'X', 'Y', 'Z', ' '
	};

	const data_type flib2[37] =
	{
		data_type({0x3C,0x42,0x42,0x42,0x42,0x42,0x42,0x3C}),		//0
		data_type({0x10,0x30,0x50,0x10,0x10,0x10,0x10,0x7C}),		//1
		data_type({0x3E,0x02,0x02,0x3E,0x20,0x20,0x3E,0x00}),		//2
		data_type({0x00,0x7C,0x04,0x04,0x7C,0x04,0x04,0x7C}),		//3
		data_type({0x08,0x18,0x28,0x48,0xFE,0x08,0x08,0x08}),		//4
		data_type({0x3C,0x20,0x20,0x3C,0x04,0x04,0x3C,0x00}),		//5
		data_type({0x3C,0x20,0x20,0x3C,0x24,0x24,0x3C,0x00}),		//6
		data_type({0x3E,0x22,0x04,0x08,0x08,0x08,0x08,0x08}),		//7
		data_type({0x00,0x3E,0x22,0x22,0x3E,0x22,0x22,0x3E}),		//8
		data_type({0x3E,0x22,0x22,0x3E,0x02,0x02,0x02,0x3E}),		//9
		data_type({0x08,0x14,0x22,0x3E,0x22,0x22,0x22,0x22}),		//A
		data_type({0x3C,0x22,0x22,0x3E,0x22,0x22,0x3C,0x00}),		//B
		data_type({0x3C,0x40,0x40,0x40,0x40,0x40,0x3C,0x00}),		//C
		data_type({0x7C,0x42,0x42,0x42,0x42,0x42,0x7C,0x00}),		//D
		data_type({0x7C,0x40,0x40,0x7C,0x40,0x40,0x40,0x7C}),		//E
		data_type({0x7C,0x40,0x40,0x7C,0x40,0x40,0x40,0x40}),		//F
		data_type({0x3C,0x40,0x40,0x40,0x40,0x44,0x44,0x3C}),		//G
		data_type({0x44,0x44,0x44,0x7C,0x44,0x44,0x44,0x44}),		//H
		data_type({0x7C,0x10,0x10,0x10,0x10,0x10,0x10,0x7C}),		//I
		data_type({0x3C,0x08,0x08,0x08,0x08,0x08,0x48,0x30}),		//J
		data_type({0x00,0x24,0x28,0x30,0x20,0x30,0x28,0x24}),		//K
		data_type({0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x7C}),		//L
		data_type({0x81,0xC3,0xA5,0x99,0x81,0x81,0x81,0x81}),		//M
		data_type({0x00,0x42,0x62,0x52,0x4A,0x46,0x42,0x00}),		//N
		data_type({0x3C,0x42,0x42,0x42,0x42,0x42,0x42,0x3C}),		//O
		data_type({0x3C,0x22,0x22,0x22,0x3C,0x20,0x20,0x20}),		//P
		data_type({0x1C,0x22,0x22,0x22,0x22,0x26,0x22,0x1D}),		//Q
		data_type({0x3C,0x22,0x22,0x22,0x3C,0x24,0x22,0x21}),		//R
		data_type({0x00,0x1E,0x20,0x20,0x3E,0x02,0x02,0x3C}),		//S
		data_type({0x00,0x3E,0x08,0x08,0x08,0x08,0x08,0x08}),		//T
		data_type({0x42,0x42,0x42,0x42,0x42,0x42,0x22,0x1C}),		//U
		data_type({0x42,0x42,0x42,0x42,0x42,0x42,0x24,0x18}),		//V
		data_type({0x00,0x49,0x49,0x49,0x49,0x2A,0x1C,0x00}),		//W
		data_type({0x00,0x41,0x22,0x14,0x08,0x14,0x22,0x41}),		//X
		data_type({0x41,0x22,0x14,0x08,0x08,0x08,0x08,0x08}),		//Y
		data_type({0x00,0x7F,0x02,0x04,0x08,0x10,0x20,0x7F}),		//Z
		data_type({0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}),		//space
	};

private:
	font_lib() = default;
	~font_lib() = default;
	font_lib(const font_lib &) = delete;
	font_lib &operator=(const font_lib &) = delete;

private:
	int findchar(char c)
	{
		for (uint8_t index = 0; index < 37; index++)
		{
			if (c == characters[index] || toupper(c) == characters[index])
			{
				return index;
			}
		}
		return 36;
	}

public:
	static font_lib &get_instance(void)
	{
		static font_lib inst;
		return inst;
	}

	data_type get(char c)
	{
		return flib2[findchar(c)];
	}
};


template<size_t w, size_t h>
class message_adapter
{
public:
	using pos_type = size_t;
	using size_type = size_t;
	using data_type = matrix<w, h>;
	using raw_data_type = array<int, h>;
	using show_func_type = add_pointer_t<void(data_type)>;
	using delay_func_type = add_pointer_t<void(size_type)>;

private:
	unique_ptr<data_type[]> raw_mats;
	map<pos_type, pos_type> pos_map;
	map<pos_type, pos_type> inplace_pos_map;
	string raw_data;
	size_type len;
	size_type dsize;

public:
	message_adapter() = delete;
	message_adapter(string str);

	template<size_type refresh_interval = 400>
	void once(show_func_type sf, delay_func_type df);
	template<size_t refresh_interval = 400, size_t loop_interval = 1000>
	void loop(size_type times, show_func_type sf, delay_func_type df);
};

template<size_t w, size_t h>
inline message_adapter<w, h>::message_adapter(string str)
{
	raw_data = str;

	len = dsize = 0;

	raw_mats = make_unique<data_type[]>(str.length()); //unique_ptr<data_type[]>{ new data_type[str.length()] };

	for (auto ch : str)
	{
		raw_mats[dsize++] = font_lib<w, h>::get_instance().get(ch);
	}

	len = dsize * w;

	for (int i = 0; i < len; i++)
	{
		pos_map[i] = i / w;
		printf("init:pos_map[%d]=%d/%d=%d\n", i, i, w, i / w);
		inplace_pos_map[i] = i % w;
		printf("init:inplace_pos_map[%d]=%d %s %d=%d\n", i, i, "%", w, i % w);
	}
}

template<size_t w, size_t h>
template<size_t refresh_interval>
inline void message_adapter<w, h>::once(show_func_type sf, delay_func_type df)
{
	for (pos_type left = 0; left < len; left++)
	{
		decltype(left) right = left + w;
		pos_type gleft = pos_map[left], gright = pos_map[right],
			inpleft = inplace_pos_map[left], inpright = inplace_pos_map[right];

		if (gleft == gright)
		{
			sf(raw_mats[gleft]);
		}
		else
		{
			auto dl = raw_mats[gleft].part_get(inpleft, data_type::PART_TYPE::RIGHT);
			auto dr = raw_mats[gright].part_get(inpright, data_type::PART_TYPE::LEFT);
			auto mat = data_type(data_type::combine(dl, dr));
			sf(mat);
		}

		df(refresh_interval);
	}
}

template<size_t w, size_t h>
template<size_t refresh_interval, size_t loop_interval>
inline void message_adapter<w, h>::loop(size_type times, show_func_type sf, delay_func_type df)
{
	while (times--)
	{
		once<refresh_interval>(sf, df);
		df(loop_interval);
	}
}

template<size_t w = 8, size_t h = 8>
class max7219
{
public:
	using data_type = matrix<w, h>;
	using message_type = std::string;
	using size_type = size_t;
private:
	max7219();
	~max7219();
	max7219(const max7219 &) = delete;
	max7219 &operator=(const max7219 &) = delete;

	static void write(write_pack p1, write_pack p2);
	static void print(data_type d);


	static inline void write_byte(uint8_t d)
	{
		bcm2835_gpio_write(Pin(Pins::CS), LOW);
		bcm2835_spi_transfer(d);
	}

public:
	static void write(write_pack p);

public:
	static max7219 &get_instance(void)
	{
		static max7219 inst;
		return inst;
	}

	static void brightness(size_type brightness = 0x03);

public:
	void draw(data_type d);
	void clean(void);
	template<size_type refresh_interval = 100>
	void message_custom_speed(message_type msg);

	void message(message_type msg);
	void bcm_delay(size_type sz);
};


template<size_t w, size_t h>
inline max7219<w, h>::max7219()
{
	int ret_init = bcm2835_init();
	if (!ret_init)
	{
		printf("Failed to initialize BCM2835\n");
		throw std::system_error(std::error_code(ret_init, std::generic_category()), "Failed to initialize BCM2835");
	}

	bcm2835_spi_begin();
	bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);
	bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);
	bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_256);
	bcm2835_gpio_fsel(Pin(Pins::CS), BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_write(0, HIGH);

	bcm2835_delay(50);

	write({ 0x09, 0x00 }, { 0x09, 0x00 });
	write({ 0x0a, 0x03 }, { 0x0a, 0x03 });
	write({ 0x0b, 0x07 }, { 0x0b, 0x07 });
	write({ 0x0c, 0x01 }, { 0x0c, 0x01 });
	write({ 0x0f, 0x00 }, { 0x0f, 0x00 });

	printf("max7219() initialized\n");
}

template<size_t w, size_t h>
inline max7219<w, h>::~max7219()
{
	this->clean();
	bcm2835_spi_end();
	bcm2835_close();
	printf("max7219() deinitialized\n");
}

template<size_t w, size_t h>
inline void max7219<w, h>::write(write_pack p)
{
	write(p, p);
}

template<size_t w, size_t h>
inline void max7219<w, h>::write(write_pack p1, write_pack p2)
{
	bcm2835_gpio_write(Pin(Pins::CS), LOW);
	//write_byte(p1.addr);
	//write_byte(p1.data);
	//write_byte(p2.addr);
	//write_byte(p2.data);
	write_byte(std::get<0>(p1));
	write_byte(std::get<1>(p1));
	write_byte(std::get<0>(p2));
	write_byte(std::get<1>(p2));

	bcm2835_gpio_write(Pin(Pins::CS), HIGH);

}

template<size_t w, size_t h>
inline void max7219<w, h>::print(data_type m)
{
	auto data = m.get_data();
	for (auto l : data)
	{
		for (int i = 0; i < 8; i++)
		{
			printf("%d ", (l >> (w - i - 1)) & 1);
		}
		printf("\n");
	}
	printf("\n\n");

	for (uint8_t i = 0; i < 8; i++)
	{
		write({ i + 1,data[i] });
	}
}

template<size_t w, size_t h>
inline void max7219<w, h>::draw(data_type d)
{
	print(d);
}

template<size_t w, size_t h>
inline void max7219<w, h>::clean(void)
{
	print(font_lib<w, h>::get_instance().get(' '));
}

template<size_t w, size_t h>
inline void max7219<w, h>::message(message_type msg)
{
	string m(msg);
	message_adapter<8, 8> mad(msg);
	mad.once<300>(this->print, bcm2835_delay);
}

template<size_t w, size_t h>
inline void max7219<w, h>::brightness(size_type brightness)
{
	brightness = std::min(brightness, 6u);
	brightness = std::max(brightness, 1u);

	uint8_t d = 0;
	for (int i = 0; i < brightness; i++)
	{
		d |= (1 << i);
	}
	max7219<8, 8>::get_instance().write({ 0x0a,d });
}

template<size_t w, size_t h>
inline void max7219<w, h>::bcm_delay(size_type sz)
{
	bcm2835_delay(sz);
}

template<size_t w, size_t h>
template<typename max7219<w, h>::size_type refresh_interval>
inline void max7219<w, h>::message_custom_speed(message_type msg)
{
	string m(msg);
	message_adapter<8, 8> mad(msg);
	mad.once<refresh_interval>(this->print, bcm2835_delay);
}
