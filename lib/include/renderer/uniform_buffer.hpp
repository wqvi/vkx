#pragma once

#include <renderer/core/allocable.hpp>

namespace vkx
{
  template <class T>
  class UniformBuffer : public Allocable<vk::Buffer>
  {
  public:
    UniformBuffer() = default;

    explicit UniformBuffer(Device const &device)
        : UniformBuffer({}, device) {}

    explicit UniformBuffer(T const &value, Device const &device)
        : device(*device)
    {
      obj = device.createBufferUnique(sizeof(T), vk::BufferUsageFlagBits::eUniformBuffer);
      memory = device.allocateMemoryUnique(obj, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
      uniformObject = value;
    }

    T *operator->()
    {
      return &uniformObject;
    }

    void mapMemory() const
    {
      void *mappedMemory = device.mapMemory(*memory, 0, sizeof(T), {});
      std::memcpy(mappedMemory, &uniformObject, sizeof(T));
      device.unmapMemory(*memory);
    }

    void mapMemory(T const &data) const
    {
      void *mappedMemory = device.mapMemory(*memory, 0, sizeof(T), {});
      std::memcpy(mappedMemory, &data, sizeof(T));
      device.unmapMemory(*memory);
    }

    void setObject(T const &value)
    {
      uniformObject = value;
    }

    vk::WriteDescriptorSet createWriteDescriptorSet(vk::DescriptorSet const &descriptorSet, std::uint32_t dstBinding) const
    {
      // This is a type safe uniform buffer. There will only be one info per type of this uniform buffer
      static vk::DescriptorBufferInfo info{};

      info = vk::DescriptorBufferInfo{
          *obj,     // buffer
          0,        // offset
          sizeof(T) // range
      };

      static vk::WriteDescriptorSet set{};

      set = vk::WriteDescriptorSet{
          descriptorSet,                      // dstSet
          dstBinding,                         // dstBinding
          0,                                  // dstArrayElement
          1,                                  // descriptorCount
          vk::DescriptorType::eUniformBuffer, // descriptorType
          nullptr,                            // pImageInfo
          &info                               // pBufferInfo
      };
      return set;
    }

  private:
    T uniformObject;
    vk::Device device;
  };

  struct MVP
  {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
  };

  DISABLE_WARNING(PADDING_WARNING)
  struct DirectionalLight
  {
    glm::vec3 position;                 // Position of the light in the world space
    alignas(16) glm::vec3 eyePosition;  // Position of the camera in the world space
    alignas(16) glm::vec4 ambientColor; // W is the intensity of the ambient light
    glm::vec3 diffuseColor;
    alignas(16) glm::vec3 specularColor;
    float constant;
    float linear;
    float quadratic;
  };
  RENABLE_WARNING

  struct Material
  {
    glm::vec3 specularColor;
    float shininess;
  };
}