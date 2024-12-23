#include "Object.h"

std::unique_ptr<Object> Object::createObject(std::shared_ptr<Model> model, std::shared_ptr<Texture> texture,
glm::vec3 position, glm::vec3 rotation, glm::vec3 scale) {
    std::unique_ptr<Object> object = std::unique_ptr<Object>(new Object());
    object->initObject(model, texture, position, rotation, scale);
    return object;
}


void Object::draw(VkCommandBuffer commandBuffer) {
    m_model->draw(commandBuffer);
}


glm::mat4 Object::getModelMatrix() {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, m_position);
    model = glm::rotate(model, glm::radians(m_rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(m_rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(m_rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, m_scale);
    return model;
}


void Object::initObject(std::shared_ptr<Model> model, std::shared_ptr<Texture> texture,
glm::vec3 position, glm::vec3 rotation, glm::vec3 scale) {
    m_model = model;
    m_texture = texture;
    m_position = position;
    m_rotation = rotation;
    m_scale = scale;
}
