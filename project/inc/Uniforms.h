#ifndef _UNIFORMS
#define _UNIFORMS

#include <cstddef>

namespace wgpu
{
	class IUniform
	{
	public:
		virtual ~IUniform() = default;

		virtual const void* GetData() const = 0;
		virtual size_t GetSize() const = 0;
	};

	template<typename T>
	class TypedUniform final : public IUniform
	{
	public:
		TypedUniform() = default;

		TypedUniform(const TypedUniform&) = delete;
		TypedUniform& operator=(const TypedUniform&) = delete;
		TypedUniform(TypedUniform&&) = delete;
		TypedUniform& operator=(TypedUniform&&) = delete;

		const void* GetData() const override
		{
			return static_cast<const void*>(&m_Value);
		}

		size_t GetSize() const override
		{
			return sizeof(T);
		}

		void SetValue(const T& value)
		{
			m_Value = value;
		}

	private:

		T m_Value{};
	};
}

#endif
