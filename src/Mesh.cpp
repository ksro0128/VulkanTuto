#include "Mesh.h"

std::shared_ptr<Mesh> Mesh::createMesh(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices) {
        std::shared_ptr<Mesh> mesh = std::shared_ptr<Mesh>(new Mesh());
        mesh->initMesh(vertices, indices);
        return mesh;
}


std::shared_ptr<Mesh> Mesh::createBox() {
    std::vector<Vertex> vertices = {
        Vertex { glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec2(0.0f, 0.0f) },
        Vertex { glm::vec3( 0.5f, -0.5f, -0.5f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec2(1.0f, 0.0f) },
        Vertex { glm::vec3( 0.5f,  0.5f, -0.5f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec2(1.0f, 1.0f) },
        Vertex { glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec2(0.0f, 1.0f) },

        Vertex { glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec2(0.0f, 0.0f) },
        Vertex { glm::vec3( 0.5f, -0.5f,  0.5f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec2(1.0f, 0.0f) },
        Vertex { glm::vec3( 0.5f,  0.5f,  0.5f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec2(1.0f, 1.0f) },
        Vertex { glm::vec3(-0.5f,  0.5f,  0.5f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec2(0.0f, 1.0f) },

        Vertex { glm::vec3(-0.5f,  0.5f,  0.5f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec2(1.0f, 0.0f) },
        Vertex { glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec2(1.0f, 1.0f) },
        Vertex { glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec2(0.0f, 1.0f) },
        Vertex { glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec2(0.0f, 0.0f) },

        Vertex { glm::vec3( 0.5f,  0.5f,  0.5f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec2(1.0f, 0.0f) },
        Vertex { glm::vec3( 0.5f,  0.5f, -0.5f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec2(1.0f, 1.0f) },
        Vertex { glm::vec3( 0.5f, -0.5f, -0.5f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec2(0.0f, 1.0f) },
        Vertex { glm::vec3( 0.5f, -0.5f,  0.5f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec2(0.0f, 0.0f) },

        Vertex { glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec2(0.0f, 1.0f) },
        Vertex { glm::vec3( 0.5f, -0.5f, -0.5f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec2(1.0f, 1.0f) },
        Vertex { glm::vec3( 0.5f, -0.5f,  0.5f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec2(1.0f, 0.0f) },
        Vertex { glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec2(0.0f, 0.0f) },

        Vertex { glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec2(0.0f, 1.0f) },
        Vertex { glm::vec3( 0.5f,  0.5f, -0.5f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec2(1.0f, 1.0f) },
        Vertex { glm::vec3( 0.5f,  0.5f,  0.5f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec2(1.0f, 0.0f) },
        Vertex { glm::vec3(-0.5f,  0.5f,  0.5f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec2(0.0f, 0.0f) },
    };

    std::vector<uint32_t> indices = {
        0,  2,  1,  2,  0,  3,
        4,  5,  6,  6,  7,  4,
        8,  9, 10, 10, 11,  8,
        12, 14, 13, 14, 12, 15,
        16, 17, 18, 18, 19, 16,
        20, 22, 21, 22, 20, 23,
    };

    return createMesh(vertices, indices);
}


std::shared_ptr<Mesh> Mesh::createSphere() {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

        uint32_t latiSegmentCount = 16;
        uint32_t longiSegmentCount = 32;

        uint32_t circleVertCount = longiSegmentCount + 1;
        vertices.resize((latiSegmentCount + 1) * circleVertCount);
        for (uint32_t i = 0; i <= latiSegmentCount; i++) {
            float v = (float)i / (float)latiSegmentCount;
            float phi = (v - 0.5f) * glm::pi<float>();
            auto cosPhi = cosf(phi);
            auto sinPhi = sinf(phi);
            for (uint32_t j = 0; j <= longiSegmentCount; j++) {
                float u = (float)j / (float)longiSegmentCount;
                float theta = u * glm::pi<float>() * 2.0f;
                auto cosTheta = cosf(theta);
                auto sinTheta = sinf(theta);
                auto point = glm::vec3(
                    cosPhi * cosTheta, sinPhi, -cosPhi * sinTheta);
                
                vertices[i * circleVertCount + j] = Vertex {
                    point * 0.5f, point, glm::vec2(u, v)
                };
            }
        }

        indices.resize(latiSegmentCount * longiSegmentCount * 6);
        for (uint32_t i = 0; i < latiSegmentCount; i++) {
            for (uint32_t j = 0; j < longiSegmentCount; j++) {
                uint32_t vertexOffset = i * circleVertCount + j;
                uint32_t indexOffset = (i * longiSegmentCount + j) * 6;
                indices[indexOffset    ] = vertexOffset;
                indices[indexOffset + 1] = vertexOffset + 1;
                indices[indexOffset + 2] = vertexOffset + 1 + circleVertCount;
                indices[indexOffset + 3] = vertexOffset;
                indices[indexOffset + 4] = vertexOffset + 1 + circleVertCount;
                indices[indexOffset + 5] = vertexOffset + circleVertCount;
            }
        }
    return createMesh(vertices, indices);
}


std::shared_ptr<Mesh> Mesh::createPlane() {
    std::vector<Vertex> vertices = {
        Vertex { glm::vec3(-0.5f, -0.5f, 0.0f), glm::vec3( 0.0f,  0.0f, 1.0f), glm::vec2(0.0f, 0.0f) },
        Vertex { glm::vec3( 0.5f, -0.5f, 0.0f), glm::vec3( 0.0f,  0.0f, 1.0f), glm::vec2(1.0f, 0.0f) },
        Vertex { glm::vec3( 0.5f,  0.5f, 0.0f), glm::vec3( 0.0f,  0.0f, 1.0f), glm::vec2(1.0f, 1.0f) },
        Vertex { glm::vec3(-0.5f,  0.5f, 0.0f), glm::vec3( 0.0f,  0.0f, 1.0f), glm::vec2(0.0f, 1.0f) },
    };

    std::vector<uint32_t> indices = {
        0,  1,  2,  2,  3,  0,
    };

    return createMesh(vertices, indices);
}


void Mesh::cleanup() {
    m_vertexBuffer->cleanup();
    m_indexBuffer->cleanup();
}


void Mesh::draw(VkCommandBuffer commandBuffer) {	
    m_vertexBuffer->bind(commandBuffer);
    m_indexBuffer->bind(commandBuffer);
    vkCmdDrawIndexed(commandBuffer, m_indexBuffer->getIndexCount(), 1, 0, 0, 0);
}


void Mesh::initMesh(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices) {
    auto& context = VulkanContext::getContext();

    m_vertexBuffer = VertexBuffer::createVertexBuffer(vertices);
    m_indexBuffer = IndexBuffer::createIndexBuffer(indices);
}
