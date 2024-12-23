#include "Scene.h"

std::unique_ptr<Scene> Scene::createScene() {
    std::unique_ptr<Scene> scene = std::unique_ptr<Scene>(new Scene());
    scene->initScene();
    return scene;
}


// object는 cleanup 해줄 필요 없지만 model과 texture는 해줘야함
void Scene::cleanup() { 
    m_boxModel->cleanup();
    m_sphereModel->cleanup();
    m_planeModel->cleanup();

    m_vikingModel->cleanup();
    m_catModel->cleanup();

    m_vikingTexture->cleanup();
    m_sampleTexture->cleanup();
    m_catTexture->cleanup();
    m_karinaTexture->cleanup();
}


glm::mat4 Scene::getViewMatrix() {
    return glm::lookAt(m_cameraPos, m_cameraPos + m_cameraFront, m_cameraUp);
}


glm::mat4 Scene::getProjMatrix(VkExtent2D swapChainExtent) {
    return glm::perspective(glm::radians(45.0f), swapChainExtent.width / (float) swapChainExtent.height, 0.1f, 100.0f);
}


void Scene::initScene() {
    m_boxModel = Model::createBoxModel();
    m_sphereModel = Model::createSphereModel();
    m_planeModel = Model::createPlaneModel();

    m_vikingModel = Model::createModel("models/viking_room.obj");
    m_catModel = Model::createModel("models/cat.obj");

    //texture 해야함
    m_vikingTexture = Texture::createTexture("textures/viking_room.png");
    m_sampleTexture = Texture::createTexture("textures/texture.png");
    m_catTexture = Texture::createTexture("textures/cat.bmp");
    m_karinaTexture = Texture::createTexture("textures/karina.jpg");

    m_objects.push_back(Object::createObject(m_boxModel, m_sampleTexture, 
    glm::vec3(-2.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f)));

    m_objects.push_back(Object::createObject(m_vikingModel, m_vikingTexture, 
    glm::vec3(2.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f)));

    m_objects.push_back(Object::createObject(m_sphereModel, m_vikingTexture, 
    glm::vec3(0.3f, 1.3f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f)));

    m_objects.push_back(Object::createObject(m_planeModel, m_karinaTexture, 
    glm::vec3(0.0f, 0.0f, -3.0f), glm::vec3(0.0f, 0.0f, 180.0f), glm::vec3(5.0f * 0.74f, 5.0f, 1.0f)));
    
    m_objects.push_back(Object::createObject(m_catModel, m_catTexture, 
    glm::vec3(0.5f, 0.0f, 0.0f), glm::vec3(-90.0f, 0.0f, 45.0f), glm::vec3(0.01f, 0.01f, 0.01f)));

    m_objectCount = m_objects.size();
}
