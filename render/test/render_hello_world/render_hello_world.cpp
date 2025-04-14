// #include <cassert>
// #include <Windows.h>
#include <memory>
#include <thread>

#include "core/win/win_app_window.h"
#include "render/resource_manager.h"
#include "render/rhi/rhi.h"
#include "render/rhi/types.h"
#include "render/rhi/shader.h"

using namespace ducklib;

struct Vertex {
    float pos[3];
    float color[4];
};

int __stdcall WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, char* cmdLine, int cmdShow) {
    WinAppWindow window{ "Hello world!", 600, 400 };
    render::Rhi rhi = {};
    render::SwapChain swap_chain = {};
    render::Adapter adapters[1] = {};
    render::Device device = {};
    render::CommandQueue queue = {};
    render::CommandList command_list = {};
    // render::Buffer v_buffer = {};
    // render::Buffer c_buffer = {};
    // render::DescriptorHeap def_descriptor_heap = {};
    render::DescriptorHeap rt_descriptor_heap = {};
    // render::BindingSetDesc binding_set_desc = {};
    // render::BindingSet binding_set = {};
    // render::Shader vertex_shader = {};
    // render::Shader pixel_shader = {};
    // render::PsoDesc pso_desc = {};
    // render::Pso pso = {};
    render::Fence fence = {};
    // render::Descriptor cb_descriptor = {};
    render::Descriptor rt_descriptors[2] = {};
    uint32_t frame_index = 0;
    ID3D12Resource* back_buffer = nullptr;

    create_rhi(rhi);
    rhi.enumerate_adapters(adapters, 1);
    rhi.create_device(adapters[0], device);
    device.create_queue(render::QueueType::GRAPHICS, queue);
    device.create_command_list(render::QueueType::GRAPHICS, command_list);

    //---------------------------------------------------------------------------------

    CD3DX12_ROOT_SIGNATURE_DESC root_signature_desc = {};
    root_signature_desc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
    ComPtr<ID3DBlob> signature;
    ComPtr<ID3D12RootSignature> root_signature;
    ComPtr<ID3DBlob> errors;
    if (FAILED(D3D12SerializeRootSignature(&root_signature_desc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &errors))) {
        std::abort();
    }
    if (FAILED(
        device.d3d12_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&root_signature)
        ))) {
        std::abort();
        }

    ComPtr<ID3DBlob> dvs;
    ComPtr<ID3DBlob> dps;
    render::Shader vs;
    render::Shader ps;
    uint32_t compiler_flags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;

    compile_shader(L"../render/test/render_hello_world/d3d_shaders.hlsl", render::ShaderType::VERTEX, "VSMain", &vs);
    compile_shader(L"../render/test/render_hello_world/d3d_shaders.hlsl", render::ShaderType::PIXEL, "PSMain", &ps);

    if (FAILED(
        D3DCompileFromFile(L"../render/test/render_hello_world/d3d_shaders.hlsl", nullptr, nullptr, "VSMain", "vs_5_0", compiler_flags, 0, &
            dvs, nullptr))) {
        std::abort();
            }
    if (FAILED(
        D3DCompileFromFile(L"../render/test/render_hello_world/d3d_shaders.hlsl", nullptr, nullptr, "PSMain", "ps_5_0", compiler_flags, 0, &
            dps, nullptr))) {
        std::abort();
            }

    D3D12_INPUT_ELEMENT_DESC ia_desc[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };

    ComPtr<ID3D12PipelineState> pso;
    render::Pso my_pso = {};
    render::PsoDesc my_pso_desc = {};
    render::BindingSet binding_set = {};
    render::BindingSetDesc binding_set_desc = {};
    D3D12_GRAPHICS_PIPELINE_STATE_DESC pso_desc = {};
    pso_desc.InputLayout = { ia_desc, 2 };
    pso_desc.pRootSignature = root_signature.Get();
    pso_desc.VS = CD3DX12_SHADER_BYTECODE(vs.d3d_bytecode_blob);
    pso_desc.PS = CD3DX12_SHADER_BYTECODE(ps.d3d_bytecode_blob);
    pso_desc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    pso_desc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    pso_desc.DepthStencilState.DepthEnable = false;
    pso_desc.DepthStencilState.StencilEnable = false;
    pso_desc.SampleMask = UINT_MAX;
    pso_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    pso_desc.NumRenderTargets = 1;
    pso_desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    pso_desc.SampleDesc.Count = 1;
    if (FAILED(device.d3d12_device->CreateGraphicsPipelineState(&pso_desc, IID_PPV_ARGS(&pso)))) {
        std::abort();
    }

    device.create_binding_set(binding_set_desc, binding_set);
    my_pso_desc.input_layout.element_count = 2;
    my_pso_desc.input_layout.elements[0] = { "POSITION", 0, 0, 0, 0, render::Format::R32G32B32_FLOAT };
    my_pso_desc.input_layout.elements[1] = { "COLOR", 0, 0, 12, 0, render::Format::R32G32B32A32_FLOAT };
    my_pso_desc.vertex_shader = &vs;
    my_pso_desc.pixel_shader = &ps;
    my_pso_desc.rt_count = 1;
    my_pso_desc.rt_formats[0] = render::Format::R8G8B8A8_UNORM;
    device.create_pso(binding_set, my_pso_desc, my_pso);

    Vertex vertices[] = {
        { { 0.0f, 0.375f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
        { { 0.25f, -0.375f, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
        { { -0.25f, -0.375f, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } }
    };
    render::Buffer v_buffer = {};
    device.create_buffer(sizeof(vertices), v_buffer, render::HeapType::UPLOAD);
    upload_buffer_data(&v_buffer, 0, vertices, sizeof(vertices));
    // ComPtr<ID3D12Resource> v_buffer;
    // auto res_desc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(vertices));
    // auto heap_prop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    // if (FAILED(
    //     device.d3d12_device->CreateCommittedResource(&heap_prop, D3D12_HEAP_FLAG_NONE, &res_desc,
    //         D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&v_buffer)))) {
    //     std::abort();
    // }
    // uint8_t* v_buffer_ptr;
    // if (FAILED(v_buffer->Map(0, nullptr, reinterpret_cast<void**>(&v_buffer_ptr)))) {
    //     std::abort();
    // }
    // memcpy(v_buffer_ptr, vertices, sizeof(vertices));
    // v_buffer->Unmap(0, nullptr);
    // D3D12_VERTEX_BUFFER_VIEW v_buffer_view = {};
    // v_buffer_view.BufferLocation = v_buffer->GetGPUVirtualAddress();
    // v_buffer_view.StrideInBytes = sizeof(Vertex);
    // v_buffer_view.SizeInBytes = sizeof(vertices);

    //------------------------------------------------------------------------------------------

    // device.create_descriptor_heap(render::DescriptorHeapType::CBV_SRV_UAV, 128, def_descriptor_heap);
    device.create_descriptor_heap(render::DescriptorHeapType::RT, 128, rt_descriptor_heap);
    device.create_fence(frame_index, fence);
    //
    rhi.create_swap_chain(queue, 600, 400, render::Format::R8G8B8A8_UNORM, window.hwnd(), swap_chain);
    rt_descriptors[0] = { .cpu_handle = rt_descriptor_heap.cpu_handle(0) };
    swap_chain.d3d12_swap_chain->GetBuffer(0, IID_PPV_ARGS(&back_buffer));
    device.create_rt_descriptor(back_buffer, nullptr, rt_descriptors[0]);
    rt_descriptors[1] = { .cpu_handle = rt_descriptor_heap.cpu_handle(1) };
    swap_chain.d3d12_swap_chain->GetBuffer(1, IID_PPV_ARGS(&back_buffer));
    device.create_rt_descriptor(back_buffer, nullptr, rt_descriptors[1]);
    // COULD IT HAVE BEEN PSODESC SAMPLWMASK???
    // float triangle_points[][3] = { { 0.0f, 0.375f, 0.0f }, { 0.25f, -0.375f, 0.0f }, { -0.25f, -0.375f, 0.0f } };
    // device.create_buffer(sizeof(triangle_points), v_buffer, render::HeapType::UPLOAD);
    // upload_buffer_data(&v_buffer, 0, triangle_points, sizeof(triangle_points));
    //
    // device.create_buffer(256, c_buffer, render::HeapType::UPLOAD);
    // float view_matrix[4][4] = {
    //     { 1.0f, 0.0f, 0.0f, 0.0f },
    //     { 0.0f, 1.0f, 0.0f, 0.0f },
    //     { 0.0f, 0.0f, 1.0f, -5.0f },
    //     { 0.0f, 0.0f, 0.0f, 1.0f }
    // };
    // float perspective_matrix[4][4] = {
    //     { 2.4142f, 0.0f, 0.0f, 0.0f },
    //     { 0.0f, 2.4142f, 0.0f, 0.0f },
    //     { 0.0f, 0.0f, -1.002f, -0.2002f },
    //     { 0.0f, 0.0f, -1.0f, 0.0f }
    // };
    // upload_buffer_data(&c_buffer, 0, view_matrix, sizeof(view_matrix));
    // upload_buffer_data(&c_buffer, sizeof(view_matrix), perspective_matrix, sizeof(view_matrix));
    // cb_descriptor = { .cpu_handle = def_descriptor_heap.cpu_handle(0), .gpu_handle = def_descriptor_heap.gpu_handle(0) };
    // device.create_cbuffer_descriptor(c_buffer, cb_descriptor);
    //
    // binding_set_desc.binding_count = 1;
    // binding_set_desc.bindings[0].type = render::BindingType::BUFFER_DESCRIPTOR;
    // binding_set_desc.bindings[0].descriptor_binding.register_space = 0;
    // binding_set_desc.bindings[0].descriptor_binding.shader_register = 0;
    // device.create_binding_set(binding_set_desc, binding_set);
    //
    // compile_shader(L"../render/test/render_hello_world/shaders.hlsl", render::ShaderType::VERTEX, "vs_main", &vertex_shader);
    // compile_shader(L"../render/test/render_hello_world/shaders.hlsl", render::ShaderType::PIXEL, "ps_main", &pixel_shader);
    //
    // pso_desc.input_layout.element_count = 1;
    // pso_desc.input_layout.elements[0].format = render::Format::R32G32B32_FLOAT;
    // pso_desc.input_layout.elements[0].semantic_name = "POSITION";
    // pso_desc.vertex_shader = &vertex_shader;
    // pso_desc.pixel_shader = &pixel_shader;
    // pso_desc.rt_count = 1;
    // pso_desc.rt_formats[0] = render::Format::R8G8B8A8_UNORM;
    // pso_desc.rasterizer.cull_mode = render::CullMode::NONE;
    //
    // device.create_pso(binding_set, pso_desc, pso);
    // TEST END

    while (window.is_open()) {
        window.process_messages();

        uint32_t rt = frame_index % 2;
        ID3D12Resource* rt_buffer = nullptr;
        swap_chain.d3d12_swap_chain->GetBuffer(rt, IID_PPV_ARGS(&rt_buffer));
        const float clear_color[] = { 0.0f, 0.2f, 0.4f, 1.0f };
        // command_list.reset();
        // TODO: Replace these with ducklib stuff and then inits and at last the vertex data and shaders, step by step.
        // command_list.d3d12_alloc->Reset();
        // command_list.d3d12_list->Reset(command_list.d3d12_alloc, pso.Get());
        // command_list.d3d12_list->SetGraphicsRootSignature(root_signature.Get());
        command_list.d3d12_alloc->Reset();
        command_list.d3d12_list->Reset(command_list.d3d12_alloc, my_pso.d3d12_pso);
        command_list.d3d12_list->SetGraphicsRootSignature(binding_set.d3d12_signature);
        // command_list.reset(&my_pso);
        // command_list.set_binding_set(binding_set);
        command_list.set_viewport(0.0f, 0.0f, 600.0f, 400.0f);
        command_list.set_scissor_rect(0.0f, 0.0f, 600.0f, 400.0f);
        command_list.resource_barrier(rt_buffer, render::ResourceState::PRESENT, render::ResourceState::RENDER_TARGET);
        command_list.set_rt(rt_descriptors[rt]);
        command_list.clear_rt(rt_descriptors[rt], clear_color);
        command_list.set_primitive_topology(render::PrimitiveTopology::TRIANGLE);
        command_list.set_vertex_buffer(v_buffer, sizeof(Vertex));
        command_list.draw(3, 0);
        command_list.resource_barrier(rt_buffer, render::ResourceState::RENDER_TARGET, render::ResourceState::PRESENT);
        command_list.close();
        queue.execute(command_list);
        swap_chain.present();
        queue.signal(fence, frame_index);
        fence.set_completion_value(frame_index);
        fence.wait();
        frame_index++;

        // TEST
        // command_list.reset();
        //
        // uint32_t rt = frame_index % 2;
        // float rand_something = (float)rand() / (float)RAND_MAX;
        // float clear_color[] = { 0.0f, 0.2f, 0.4f, 1.0f };
        // ID3D12Resource* rt_buffer = nullptr;
        // swap_chain.d3d12_swap_chain->GetBuffer(rt, IID_PPV_ARGS(&rt_buffer));
        //
        // command_list.set_pso(pso);
        // command_list.set_binding_set(binding_set);
        // command_list.resource_barrier(rt_buffer, render::ResourceState::PRESENT, render::ResourceState::RENDER_TARGET);
        // command_list.set_rt(rt_descriptors[rt]);
        // command_list.clear_rt(rt_descriptors[rt], clear_color);
        //
        // command_list.set_viewport(600.0f, 400.0f);
        // command_list.set_scissor_rect(0.0f, 0.0f, 600.0f, 400.0f);
        // command_list.set_primitive_topology(render::PrimitiveTopology::TRIANGLE);
        // command_list.set_vertex_buffer(v_buffer, 12);
        // command_list.draw(3, 0);
        //
        // command_list.resource_barrier(rt_buffer, render::ResourceState::RENDER_TARGET, render::ResourceState::PRESENT);
        //
        // command_list.close();
        // queue.execute(command_list);
        //
        // swap_chain.present();
        // queue.signal(fence, frame_index);
        // fence.set_completion_value(frame_index);
        // frame_index++;
        // fence.wait();

        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        // TEST END
    }

    return 0;
}
