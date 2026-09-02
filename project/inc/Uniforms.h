#ifndef _UNIFORMS
#define _UNIFORMS

namespace wgpu
{
	class IUniform
	{
	public:
		virtual ~IUniform() = default;

		virtual void* GetData() const = 0;
		virtual size_t GetSize() const = 0;
	};

	template<typename T>
	class UniformType final : public IUniform
	{
	public:
		UniformType() = default;

		UniformType(const UniformType&) = delete;
		UniformType& operator=(const UniformType&) = delete;
		UniformType(UniformType&&) = delete;
		UniformType& operator=(UniformType&&) = delete;

		void* GetData() const override
		{
			return static_cast<void*>(&m_Value);
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
