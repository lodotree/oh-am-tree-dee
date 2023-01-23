#include <glad/glad.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>

#include <graphics.h>
#include <SceneView.h>
#include <Texture.h>
#include <Framebuffer.h>
#include <ImGuiRenderer.h>

#include <imgui/imgui.h>


using namespace OM3D;

static float delta_time = 0.0f;
const glm::uvec2 window_size(1600, 900);


void glfw_check(bool cond) {
    if(!cond) {
        const char* err = nullptr;
        glfwGetError(&err);
        std::cerr << "GLFW error: " << err << std::endl;
        std::exit(EXIT_FAILURE);
    }
}

void update_delta_time() {
    static double time = 0.0;
    const double new_time = program_time();
    delta_time = float(new_time - time);
    time = new_time;
}

void process_inputs(GLFWwindow* window, Camera& camera) {
    static glm::dvec2 mouse_pos;

    glm::dvec2 new_mouse_pos;
    glfwGetCursorPos(window, &new_mouse_pos.x, &new_mouse_pos.y);

    {
        glm::vec3 movement = {};
        if(glfwGetKey(window, 'W') == GLFW_PRESS) {
            movement += camera.forward();
        }
        if(glfwGetKey(window, 'S') == GLFW_PRESS) {
            movement -= camera.forward();
        }
        if(glfwGetKey(window, 'D') == GLFW_PRESS) {
            movement += camera.right();
        }
        if(glfwGetKey(window, 'A') == GLFW_PRESS) {
            movement -= camera.right();
        }

        float speed = 10.0f;
        if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
            speed *= 10.0f;
        }
        if(glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
            speed *= 50.0f;
        }

        if(glm::length(movement) > 0.0f) {
            const glm::vec3 new_pos = camera.position() + movement * delta_time * speed;
            camera.set_view(glm::lookAt(new_pos, new_pos + camera.forward(), camera.up()));
        }
    }

    if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        const glm::vec2 delta = glm::vec2(mouse_pos - new_mouse_pos) * 0.01f;
        if(glm::length(delta) > 0.0f) {
            glm::mat4 rot = glm::rotate(glm::mat4(1.0f), delta.x, glm::vec3(0.0f, 1.0f, 0.0f));
            rot = glm::rotate(rot, delta.y, camera.right());
            camera.set_view(glm::lookAt(camera.position(), camera.position() + (glm::mat3(rot) * camera.forward()), (glm::mat3(rot) * camera.up())));
        }

    }

    mouse_pos = new_mouse_pos;
}

Result<Scene> load_scene(const std::string& path = "cube.glb", bool chunky = false){
    // Load default cube model
    auto result = Scene::from_gltf(std::string(data_path) + path, chunky);
    if(!result.is_ok) return {false, {}};
    auto scene = std::move(result.value);

    // Add lights
    {
        PointLight light;
        light.set_position(glm::vec3(1.0f, 2.0f, 4.0f));
        light.set_color(glm::vec3(0.0f, 10.0f, 0.0f));
        light.set_radius(100.0f);
        scene.add_object(std::move(light));
    }
    {
        PointLight light;
        light.set_position(glm::vec3(1.0f, 2.0f, -4.0f));
        light.set_color(glm::vec3(10.0f, 0.0f, 0.0f));
        light.set_radius(50.0f);
        scene.add_object(std::move(light));
    }

    return {true, std::move(scene) };
}


int main(int, char**) {
    DEBUG_ASSERT([] { std::cout << "Debug asserts enabled" << std::endl; return true; }());

    glfw_check(glfwInit());
    DEFER(glfwTerminate());

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(window_size.x, window_size.y, "TP window", nullptr, nullptr);
    glfw_check(window);
    DEFER(glfwDestroyWindow(window));

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync
    init_graphics();

    ImGuiRenderer imgui(window);
    char imgui_scene_name_buffer[1024] = {};
    bool imgui_gen_scene_chunking = false;
    bool imgui_scene_force_fullres = false;

    auto lodg = GPULODGen();

    Scene scene;
    {
        auto scene_k = load_scene();
        ALWAYS_ASSERT(scene_k.is_ok, "Unable to load default scene");
        scene = std::move(scene_k.value);
    }
    auto scene_up = scene.upload();
    SceneView scene_view(scene_up.get());

    auto tonemap_program = Program::from_file("tonemap.comp");
    auto zpass_program = Program::from_files("zpass.frag", "zpass.vert");
    auto screen_program = Program::from_files("screen.frag", "screen.vert");

    Texture depth(window_size, ImageFormat::Depth32_FLOAT);
    Texture albedo(window_size, ImageFormat::RGBA8_sRGB);
    Texture normal(window_size, ImageFormat::RGBA8_UNORM);
    Texture lit(window_size, ImageFormat::RGBA16_FLOAT);
    Texture color(window_size, ImageFormat::RGBA8_UNORM);
    // Framebuffer z_framebuffer(&depth);
    Framebuffer g_framebuffer(&depth, std::array{&albedo, &normal});
    Framebuffer main_framebuffer(nullptr, std::array{&lit});
    Framebuffer tonemap_framebuffer(nullptr, std::array{&color});

    for(;;) {
        StaticMeshUploaded::DEBUG_VERTS_DRAWN = 0;
        StaticMeshUploaded::DEBUG_TRIS_DRAWN = 0;

        glfwPollEvents();
        if(glfwWindowShouldClose(window) || glfwGetKey(window, GLFW_KEY_ESCAPE)) {
            break;
        }

        update_delta_time();

        if(const auto& io = ImGui::GetIO(); !io.WantCaptureMouse && !io.WantCaptureKeyboard) {
            process_inputs(window, scene_view.camera());
        }

        // Render the scene
        {
            // Z-pass
            // z_framebuffer.bind();
            // zpass_program->bind();
            // scene_view.render(false);
            // G-pass
            g_framebuffer.bind();
            scene_view.render(true, imgui_scene_force_fullres);
            // Screen
            glDisable(GL_DEPTH_TEST);
            screen_program->bind();
			scene_view.bind_lights();
            main_framebuffer.bind();
            albedo.bind(0);
            normal.bind(1);
            depth.bind(2);
            glDrawArrays(GL_TRIANGLES, 0, 3);
            glEnable(GL_DEPTH_TEST);
            // // Legacy forward pass
            // main_framebuffer.bind();
            // scene_view.render(true);
        }

        // Apply a tonemap in compute shader
        {
            tonemap_program->bind();
            lit.bind(0);
            color.bind_as_image(1, AccessType::WriteOnly);
            glDispatchCompute(align_up_to(window_size.x, 8), align_up_to(window_size.y, 8), 1);
        }
        // Blit tonemap result to screen
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        tonemap_framebuffer.blit();

        // GUI
        imgui.start();
        {
            ImGui::Text("%llu tris, %llu vertices", StaticMeshUploaded::DEBUG_TRIS_DRAWN, StaticMeshUploaded::DEBUG_VERTS_DRAWN);
            if(ImGui::Checkbox("Generate chunky clustering", &imgui_gen_scene_chunking));
            if(ImGui::InputText("Load scene", imgui_scene_name_buffer, sizeof(imgui_scene_name_buffer), ImGuiInputTextFlags_EnterReturnsTrue)) {
                auto result = load_scene(imgui_scene_name_buffer, imgui_gen_scene_chunking);
                if(!result.is_ok) {
                    std::cerr << "Unable to load scene (" << imgui_scene_name_buffer << ")" << std::endl;
                } else {
                    scene_up = nullptr;
                    scene = std::move(result.value);
                    scene_up = scene.upload();
                    scene_view = scene_view.transfer(scene_up.get());
                }
            }
            if(ImGui::Checkbox("Force full resolution", &imgui_scene_force_fullres));
            if(ImGui::Checkbox("Debug chunky clusters", &ChunkUploaded::DEBUG_CLUSTER_STATUS));
        }
        imgui.finish();

        glfwSwapBuffers(window);
    }

    scene_up = nullptr; // destroy scene and child OpenGL objects
}
