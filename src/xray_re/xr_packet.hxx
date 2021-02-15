#pragma once

#include "xr_types.hxx"

#include <string>
#include <vector>

namespace xray_re
{
	class xr_packet
	{
		public:
			xr_packet();

			enum
			{
				BUFFER_SIZE = 0x1000,
			};

			void clear();

			template<typename T> void w(const T& value);
			template<typename T> void w_cseq(size_t n, const T values[]);
			template<typename T> void w_seq(const T& container);
			void w_raw(const void *data, size_t size);
			void w_sz(const std::string& value);
			void w_u64(uint64_t value);
			void w_s64(int64_t value);
			void w_u32(uint32_t value);
			void w_s32(int32_t value);
			void w_u24(uint32_t value);
			void w_u16(uint16_t value);
			void w_s16(int16_t value);
			void w_u8(uint8_t value);
			void w_s8(int8_t value);
			void w_bool(bool value);
			void w_float(float value);
			void w_float_q16(float value, float min = 0, float max = 1.f);
			void w_float_q8(float value, float min = 0, float max = 1.f);
			void w_size_u32(size_t size);
			void w_size_u16(size_t size);
			void w_size_u8(size_t size);
			void w_angle16(float value);
			void w_angle8(float value);
			size_t w_tell() const;
			void w_seek(size_t pos);
			void w_begin(uint16_t id);

			const char* skip_sz();

			template<typename T> void r(T& value);
			template<typename T> T r();
			template<typename T> void r_cseq(size_t n, T values[]);
			template<typename T> void r_seq(size_t n, T& container);
			void r_raw(void *data, size_t size);
			void r_sz(std::string& value);
			uint64_t r_u64();
			void r_u64(uint64_t& value);
			void r_s64(int64_t& value);
			uint32_t r_u32();
			void r_u32(uint32_t& value);
			int32_t r_s32();
			void r_s32(int32_t& value);
			uint32_t r_u24();
			void r_u24(uint32_t& value);
			uint16_t r_u16();
			void r_u16(uint16_t& value);
			int16_t r_s16();
			void r_s16(int16_t& value);
			uint8_t r_u8();
			void r_u8(uint8_t& value);
			int8_t r_s8();
			void r_s8(int8_t& value);
			bool r_bool();
			void r_bool(bool& value);
			float r_float();
			void r_float(float& value);
			void r_float_q16(float& value, float min = 0, float max = 1.f);
			void r_float_q8(float& value, float min = 0, float max = 1.f);
			float r_angle8();
			void r_angle8(float& value);
			float r_angle16();
			void r_angle16(float& value);
			size_t r_tell() const;
			void r_seek(size_t pos);
			void r_advance(size_t ofs);
			void r_begin(uint16_t& id);
			bool r_eof() const;

			void init(const uint8_t *data, size_t size);
			const uint8_t* buf() const;

		private:
			uint8_t m_buf[BUFFER_SIZE];
			size_t m_w_pos;
			size_t m_r_pos;
	};

	inline const uint8_t* xr_packet::buf() const { return &m_buf[0]; }
	inline void xr_packet::clear() { m_w_pos = 0; m_r_pos = 0; }

	template<typename T> inline void xr_packet::w(const T& value) { w_raw(&value, sizeof(T)); }
	inline void xr_packet::w_u64(uint64_t value) { w<uint64_t>(value); }
	inline void xr_packet::w_s64(int64_t value) { w<int64_t>(value); }
	inline void xr_packet::w_u32(uint32_t value) { w<uint32_t>(value); }
	inline void xr_packet::w_s32(int32_t value) { w<int32_t>(value); }
	inline void xr_packet::w_u16(uint16_t value) { w<uint16_t>(value); }
	inline void xr_packet::w_s16(int16_t value) { w<int16_t>(value); }
	inline void xr_packet::w_u8(uint8_t value) { w<uint8_t>(value); }
	inline void xr_packet::w_s8(int8_t value) { w<int8_t>(value); }
	inline void xr_packet::w_bool(bool value) { w<uint8_t>(value ? 1 : 0); }
	inline void xr_packet::w_float(float value) { w<float>(value); }
	inline void xr_packet::w_float_q8(float value, float min, float max) { w_u8(uint8_t((value - min)*255.f/(max - min))); }
	inline size_t xr_packet::w_tell() const { return m_w_pos; }
	inline void xr_packet::w_seek(size_t pos) { m_w_pos = pos; assert(pos < sizeof(m_buf)); }
	inline void xr_packet::w_size_u32(size_t value) { w_u32(static_cast<uint32_t>(value & UINT32_MAX)); }
	inline void xr_packet::w_size_u16(size_t value) { w_u16(static_cast<uint16_t>(value & UINT16_MAX)); }
	inline void xr_packet::w_size_u8(size_t value) { w_u8(static_cast<uint8_t>(value & UINT8_MAX)); }

	template<typename T> inline T xr_packet::r() { T value; r_raw(&value, sizeof(T)); return value; }
	template<typename T> inline void xr_packet::r(T& value) { r_raw(&value, sizeof(T)); }
	inline uint64_t xr_packet::r_u64() { return r<uint64_t>(); }
	inline void xr_packet::r_u64(uint64_t& value) { r(value); }
	inline void xr_packet::r_s64(int64_t& value) { r(value); }
	inline uint32_t xr_packet::r_u32() { return r<uint32_t>(); }
	inline void xr_packet::r_u32(uint32_t& value) { r(value); }
	inline int32_t xr_packet::r_s32() { return r<int32_t>(); }
	inline void xr_packet::r_s32(int32_t& value) { r(value); }
	inline uint16_t xr_packet::r_u16() { return r<uint16_t>(); }
	inline void xr_packet::r_u16(uint16_t& value) { r(value); }
	inline int16_t xr_packet::r_s16() { return r<int16_t>(); }
	inline void xr_packet::r_s16(int16_t& value) { r(value); }
	inline uint8_t xr_packet::r_u8() { return r<uint8_t>(); }
	inline void xr_packet::r_u8(uint8_t& value) { r(value); }
	inline int8_t xr_packet::r_s8() { return r<int8_t>(); }
	inline void xr_packet::r_s8(int8_t& value) { r(value); }
	inline bool xr_packet::r_bool() { return r_u8() != 0; }
	inline void xr_packet::r_bool(bool& value) { value = r_u8() != 0; }
	inline float xr_packet::r_float() { return r<float>(); }
	inline void xr_packet::r_float(float& value) { r(value); }
	inline void xr_packet::r_float_q8(float& value, float min, float max) { value = r_u8()*(max - min)/255.f + min; }
	inline size_t xr_packet::r_tell() const { return m_r_pos; }
	inline void xr_packet::r_seek(size_t pos) { m_r_pos = pos; /*assert(pos < m_w_pos);*/ }
	inline void xr_packet::r_advance(size_t ofs) { m_r_pos += ofs; /*assert(m_r_pos < m_w_pos);*/ }
	inline bool xr_packet::r_eof() const { return m_r_pos == m_w_pos; }

	template<typename T> inline void xr_packet::r_seq(size_t n, T& container)
	{
		typename T::const_pointer p = reinterpret_cast<typename T::const_pointer>(m_buf + m_r_pos);
		container.reserve(n);
		container.assign(p, p + n);
		r_advance(n*sizeof(typename T::value_type));
	}

	template<typename T> inline void xr_packet::r_cseq(size_t n, T values[])
	{
		for(T *p = values, *end = p + n; p != end; ++p)
		{
			r<T>(*p);
		}
	}

	template<typename T> inline void xr_packet::w_cseq(size_t n, const T values[])
	{
		if(n)
		{
			w_raw(values, n*sizeof(T));
		}
	}

	template<typename T> inline void xr_packet::w_seq(const T& container)
	{
		if(!container.empty())
		{
			w_raw(&container[0], container.size()*sizeof(typename T::value_type));
		}
	}
}
