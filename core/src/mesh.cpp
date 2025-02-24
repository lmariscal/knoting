#include <knoting/asset_manager.h>
#include <knoting/log.h>
#include <knoting/mesh.h>
#include <filesystem>
#include <fstream>

using namespace std::chrono;

namespace knot {
namespace components {

Mesh::Mesh() : Asset{AssetType::Mesh, ""} {}
Mesh::Mesh(const std::string& path) : Asset{AssetType::Mesh, path} {}

Mesh::~Mesh() {}

void Mesh::on_awake() {
    if (m_assetState != AssetState::Finished) {
        m_assetState = AssetState::Loading;
        internal_load_obj(m_fullPath);
    }
}

void Mesh::on_destroy() {
    if (isValid(m_vbh)) {
        bgfx::destroy(m_vbh);
    }
    if (isValid(m_ibh)) {
        bgfx::destroy(m_ibh);
    }
    log::info("removed mesh : {}", m_fullPath);
}

void Mesh::create_cube() {
    std::vector<unsigned int> tempIndex = {
        0,  2,  1,  1,  2,  3,  4,  5,  6,  5,  7,  6,

        8,  10, 9,  9,  10, 11, 12, 13, 14, 13, 15, 14,

        16, 18, 17, 17, 18, 19, 20, 21, 22, 21, 23, 22,
    };

    m_indexBuffer = std::make_shared<IndexBuffer>();
    m_indexBuffer->set_index_buffer(tempIndex);

    m_ibh = bgfx::createIndexBuffer(bgfx::makeRef(&m_indexBuffer->get_index_start(), m_indexBuffer->get_memory_size()));

    VertexLayout::init();
    m_vertexLayout = {
        {-1.0f, 1.0f, 1.0f, encode_normal_rgba8(0.0f, 0.0f, 1.0f), 0, 0, 0},
        {1.0f, 1.0f, 1.0f, encode_normal_rgba8(0.0f, 0.0f, 1.0f), 0, 1, 0},
        {-1.0f, -1.0f, 1.0f, encode_normal_rgba8(0.0f, 0.0f, 1.0f), 0, 0, 1},
        {1.0f, -1.0f, 1.0f, encode_normal_rgba8(0.0f, 0.0f, 1.0f), 0, 1, 1},
        {-1.0f, 1.0f, -1.0f, encode_normal_rgba8(0.0f, 0.0f, -1.0f), 0, 0, 0},
        {1.0f, 1.0f, -1.0f, encode_normal_rgba8(0.0f, 0.0f, -1.0f), 0, 1, 0},
        {-1.0f, -1.0f, -1.0f, encode_normal_rgba8(0.0f, 0.0f, -1.0f), 0, 0, 1},
        {1.0f, -1.0f, -1.0f, encode_normal_rgba8(0.0f, 0.0f, -1.0f), 0, 1, 1},
        {-1.0f, 1.0f, 1.0f, encode_normal_rgba8(0.0f, 1.0f, 0.0f), 0, 0, 0},
        {1.0f, 1.0f, 1.0f, encode_normal_rgba8(0.0f, 1.0f, 0.0f), 0, 1, 0},
        {-1.0f, 1.0f, -1.0f, encode_normal_rgba8(0.0f, 1.0f, 0.0f), 0, 0, 1},
        {1.0f, 1.0f, -1.0f, encode_normal_rgba8(0.0f, 1.0f, 0.0f), 0, 1, 1},
        {-1.0f, -1.0f, 1.0f, encode_normal_rgba8(0.0f, -1.0f, 0.0f), 0, 0, 0},
        {1.0f, -1.0f, 1.0f, encode_normal_rgba8(0.0f, -1.0f, 0.0f), 0, 1, 0},
        {-1.0f, -1.0f, -1.0f, encode_normal_rgba8(0.0f, -1.0f, 0.0f), 0, 0, 1},
        {1.0f, -1.0f, -1.0f, encode_normal_rgba8(0.0f, -1.0f, 0.0f), 0, 1, 1},
        {1.0f, -1.0f, 1.0f, encode_normal_rgba8(1.0f, 0.0f, 0.0f), 0, 0, 0},
        {1.0f, 1.0f, 1.0f, encode_normal_rgba8(1.0f, 0.0f, 0.0f), 0, 1, 0},
        {1.0f, -1.0f, -1.0f, encode_normal_rgba8(1.0f, 0.0f, 0.0f), 0, 0, 1},
        {1.0f, 1.0f, -1.0f, encode_normal_rgba8(1.0f, 0.0f, 0.0f), 0, 1, 1},
        {-1.0f, -1.0f, 1.0f, encode_normal_rgba8(-1.0f, 0.0f, 0.0f), 0, 0, 0},
        {-1.0f, 1.0f, 1.0f, encode_normal_rgba8(-1.0f, 0.0f, 0.0f), 0, 1, 0},
        {-1.0f, -1.0f, -1.0f, encode_normal_rgba8(-1.0f, 0.0f, 0.0f), 0, 0, 1},
        {-1.0f, 1.0f, -1.0f, encode_normal_rgba8(-1.0f, 0.0f, 0.0f), 0, 1, 1},
    };

    m_vbh =
        bgfx::createVertexBuffer(bgfx::makeRef(&m_vertexLayout[0], sizeof(m_vertexLayout[0]) * m_vertexLayout.size()),
                                 VertexLayout::s_meshVertexLayout);
}

void Mesh::generate_default_asset() {
    m_assetState = AssetState::Loading;
    m_fullPath = "fallbackMesh";
    create_cube();
    if (!bgfx::isValid(m_vbh)) {
        log::error("VBH : fallbackMesh failed to be created");
        m_vbh = BGFX_INVALID_HANDLE;
        m_assetState = AssetState::Failed;
        return;
    }

    if (!bgfx::isValid(m_ibh)) {
        log::error("IBH : fallbackMesh failed to be created");
        m_ibh = BGFX_INVALID_HANDLE;
        m_assetState = AssetState::Failed;
        return;
    }

    m_assetState = AssetState::Finished;
    log::info("Fallback created");
}

std::vector<std::string> Mesh::split(std::string s, const std::string& t) {
    m_splitResult.clear();
    while (!s.empty()) {
        size_t pos = s.find(t);
        if (pos == std::string::npos) {
            m_splitResult.push_back(s);
            break;
        }
        m_splitResult.push_back(s.substr(0, pos));
        s = s.substr(pos + 1, s.size() - pos - 1);
    }
    return m_splitResult;
}

bool Mesh::internal_load_obj(const std::string& path) {
    std::filesystem::path fsPath = AssetManager::get_resources_path().append(PATH_MODELS).append(path);

    if (!exists(fsPath)) {
        log::error("{} - does not Exist", fsPath.string());
        m_assetState = AssetState::Failed;
        return false;
    }
    auto start = high_resolution_clock::now();

    if (fsPath.extension().string() == ".obj") {
        std::ifstream fin(fsPath, std::ios::in);
        if (!fin) {
            log::error("cant open file {}", fsPath.string());
            m_vbh = BGFX_INVALID_HANDLE;
            m_ibh = BGFX_INVALID_HANDLE;
            m_assetState = AssetState::Failed;
            return false;
        }

        std::vector<uint32_t> vertexIndices, uvIndices, normalIndices;
        glm::vec3 tempVertex;
        glm::vec2 tempUV;
        glm::vec3 tempNormal;
        std::vector<glm::vec3> tempVertices;
        std::vector<glm::vec2> tempUVs;
        std::vector<glm::vec3> tempNormals;
        int vertexIndex, uvIndex, normalIndex;  // v/vt/vn
        int splitIndex;

        log::info("loading file {}", fsPath.string());
        std::string faceData;

        std::vector<std::string> splitData;

        std::string lineBuffer;
        std::string cmd;

        while (std::getline(fin, lineBuffer)) {
            std::stringstream ss(lineBuffer);
            ss >> cmd;
            splitIndex = 0;
            if (cmd == "v") {
                while (splitIndex < 3 && ss >> tempVertex[splitIndex]) {
                    splitIndex++;
                }
                tempVertices.emplace_back(tempVertex);

            } else if (cmd == "vt") {
                while (splitIndex < 2 && ss >> tempUV[splitIndex]) {
                    splitIndex++;
                }
                tempUVs.emplace_back(tempUV);

            } else if (cmd == "vn") {
                while (splitIndex < 3 && ss >> tempNormal[splitIndex]) {
                    splitIndex++;
                }

                tempNormal = glm::normalize(tempNormal);
                tempNormals.emplace_back(tempNormal);

            } else if (cmd == "f") {
                while (ss >> faceData) {
                    splitData = split(faceData, "/");
                    sscanf(splitData[0].c_str(), "%d", &vertexIndex);
                    vertexIndices.emplace_back(vertexIndex);

                    sscanf(splitData[1].c_str(), "%d", &uvIndex);
                    uvIndices.emplace_back(uvIndex);

                    sscanf(splitData[2].c_str(), "%d", &normalIndex);
                    normalIndices.emplace_back(normalIndex);
                }
            }
        }

        fin.close();

        // clang-format off
        VertexLayout::init();

        for (unsigned int i = 0; i < vertexIndices.size(); i++) {

            tempVertex = tempVertices[vertexIndices[i] - 1];
            tempNormal = tempNormals[normalIndices[i] - 1];
            tempUV = tempUVs[uvIndices[i] - 1];

            m_vertexLayout.emplace_back(
                VertexLayout{
                tempVertex.x,
                tempVertex.y,
                tempVertex.z,
                encode_normal_rgba8(tempNormal.x, tempNormal.y, tempNormal.z),
                0,
                tempUV.x,
                tempUV.y
                }
            );
        }
    }

    m_ibh = BGFX_INVALID_HANDLE;
    m_vbh = bgfx::createVertexBuffer(
    bgfx::makeRef(
        &m_vertexLayout[0],
        sizeof(m_vertexLayout[0]) * m_vertexLayout.size()),
        VertexLayout::s_meshVertexLayout
        );

    log::debug("init buffers {}", fsPath.string());

    // clang-format on

    m_assetState = AssetState::Finished;
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(stop - start);
    log::debug("Time taken to load : {} - {} ms ", path, duration.count());

    return true;
}

}  // namespace components
}  // namespace knot
