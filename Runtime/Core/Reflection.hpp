#pragma once
#include "Core/Defines.h"
#include "refl-cpp/include/refl.hpp"
#include "Engine/Types.h"
#include "Memory/ObjectPtr.hpp"
#include "Components/Component.h"
#include "YamlSerializable.h"

namespace Sailor
{
	class TypeInfo
	{
	public:

		// instances can be obtained only through calls to Get()
		template <typename T>
		static const TypeInfo& Get()
		{
			// here we create the singleton instance for this particular type
			static const TypeInfo ti(refl::reflect<T>());
			return ti;
		}

		const std::string& Name() const
		{
			return m_name;
		}

	private:

		// were only storing the name for demonstration purposes,
		// but this can be extended to hold other properties as well
		std::string m_name;

		// given a type_descriptor, we construct a TypeInfo
		// with all the metadata we care about (currently only name)
		template <typename T, typename... Fields>
		TypeInfo(refl::type_descriptor<T> td)
			: m_name(td.name)
		{
		}

	};

	class ReflectionInfo : public IYamlSerializable
	{
	public:

		virtual YAML::Node Serialize() const
		{
			assert(m_typeInfo);

			YAML::Node res{};
			res["typename"] = m_typeInfo->Name();
			res["members"] = m_members;

			return res;
		};

		virtual void Deserialize(const YAML::Node& inData) { assert(0); }

		const TypeInfo& GetTypeInfo() const { return *m_typeInfo; }
		const TVector<YAML::Node>& GetMembers() const { return m_members; }

	private:

		// TODO: Rethink the approach
		const TypeInfo* m_typeInfo{};
		TVector<YAML::Node> m_members{};

		friend class Reflection;
	};

	class SAILOR_API IReflectable
	{
	public:
		virtual const TypeInfo& GetTypeInfo() const = 0;
		virtual ReflectionInfo GetReflectionInfo() const = 0;
	};

#define SAILOR_REFLECTABLE() \
    virtual const TypeInfo& GetTypeInfo() const override \
    { \
        return TypeInfo::Get<::refl::trait::remove_qualifiers_t<decltype(*this)>>(); \
    } \
	virtual ReflectionInfo GetReflectionInfo() const override \
	{ \
		TypeInfo typeInfo = TypeInfo::Get<::refl::trait::remove_qualifiers_t<decltype(*this)>>(); \
		ReflectionInfo res = Reflection::Reflect<::refl::trait::remove_qualifiers_t<decltype(*this)>>(this); \
		return res; \
	} \

	class SAILOR_API Reflection
	{
	public:

		template<typename T>
		static ReflectionInfo Reflect(const T* ptr) requires IsBaseOf<IReflectable, T>
		{
			ReflectionInfo reflection;
			reflection.m_typeInfo = &ptr->GetTypeInfo();

			for_each(refl::reflect(*ptr).members, [&](auto member)
				{
					if constexpr (is_readable(member))// && refl::descriptor::has_attribute<serializable>(member))
					{
						YAML::Node node{};

						if constexpr (Sailor::IsEnum<decltype(member(*ptr))>)
						{
							node[get_display_name(member)] = SerializeEnum<decltype(member(*ptr))>(member(*ptr));
						}
						else
						{
							node[get_display_name(member)] = member(*ptr);
						}
						reflection.m_members.Add(node);
					}
				});


			return reflection;
		}
	};
}
