//#include"offscreenrendering.h"
//
//offscreenrenderering::offscreenrenderer() = default;
//offscreenrenderering::~offscreenrenderer() = default;
//
//void offscreenrenderering::initialize(directxcommon* dxcommon, int width, int height) {
//    dxcommon_ = dxcommon;
//    width_ = width;
//    height_ = height;
//
//    // テクスチャリソース作成
//    rendertexture_ = dxcommon_->createrendertargetresource(
//        dxcommon_->getdevice(), width, height,
//        dxgi_format_r8g8b8a8_unorm_srgb, { 0,0,0,1 }
//    );
//
//    // rtv作成
//    rtvhandle_ = dxcommon_->getrtvdescriptorheap()->getcpudescriptorhandleforheapstart();
//    dxcommon_->getdevice()->createrendertargetview(rendertexture_.get(), nullptr, rtvhandle_);
//
//    // srv作成
//    d3d12_shader_resource_view_desc srvdesc = {};
//    srvdesc.format = dxgi_format_r8g8b8a8_unorm_srgb;
//    srvdesc.viewdimension = d3d12_srv_dimension_texture2d;
//    srvdesc.shader4componentmapping = d3d12_default_shader_4_component_mapping;
//    srvdesc.texture2d.miplevels = 1;
//    srvhandle_ = dxcommon_->getsrvgpudescriptorhandle(0);
//    dxcommon_->getdevice()->createshaderresourceview(rendertexture_.get(), &srvdesc, dxcommon_->getsrvcpudescriptorhandle(0));
//}
//
//void offscreenrenderer::begin() {
//    // バリア: srv→rtv
//    dxcommon_->transitionsrvtorendertarget(rendertexture_.get());
//    // rtvセット
//    dxcommon_->getcommandlist()->omsetrendertargets(1, &rtvhandle_, false, nullptr);
//    // クリア
//    float clearcolor[4] = { 0, 0, 0, 1 };
//    dxcommon_->getcommandlist()->clearrendertargetview(rtvhandle_, clearcolor, 0, nullptr);
//    // ビューポート/シザー
//    dxcommon_->getcommandlist()->rssetviewports(1, &dxcommon_->getviewport());
//    dxcommon_->getcommandlist()->rssetscissorrects(1, &dxcommon_->getscissorrect());
//}
//
//void offscreenrenderer::end() {
//    // バリア: rtv→srv
//    dxcommon_->transitionrendertargettosrv(rendertexture_.get());
//}
//
//void offscreenrenderer::drawcopy(id3d12pipelinestate* pso, id3d12rootsignature* rootsig, d3d12_gpu_descriptor_handle srvhandle) {
//    auto* cmd = dxcommon_->getcommandlist().get();
//    // ディスクリプタヒープ
//    id3d12descriptorheap* heaps[] = { dxcommon_->getsrvdescriptorheap().get() };
//    cmd->setdescriptorheaps(1, heaps);
//    // pso/ルートシグネチャ
//    cmd->setgraphicsrootsignature(rootsig);
//    cmd->setpipelinestate(pso);
//    // srvバインド（ルートパラメータ0想定）
//    cmd->setgraphicsrootdescriptortable(0, srvhandle);
//    // トポロジ
//    cmd->iasetprimitivetopology(d3d_primitive_topology_trianglelist);
//    // drawcall
//    cmd->drawinstanced(3, 1, 0, 0);
//}
//
//d3d12_gpu_descriptor_handle offscreenrenderer::getsrvhandle() const {
//    return srvhandle_;
//}
//
//id3d12resource* offscreenrenderer::getresource() const {
//    return rendertexture_.get();
//}
