#include <memory>

#include <CxxPtr/GlibPtr.h>
#include <CxxPtr/libwebsocketsPtr.h>

#include "Helpers/LwsLog.h"
#include "Http/Log.h"
#include "Http/HttpServer.h"
#include "Signalling/Log.h"
#include "Signalling/WsServer.h"
#include "Signalling/ServerSession.h"
#include "RtStreaming/GstRtStreaming/LibGst.h"
#include "RtStreaming/GstRtStreaming/GstReStreamer.h"
#include "RtStreaming/GstRtStreaming/GstReStreamer2.h"

// #define USE_RE_STREAMER_2

static std::unique_ptr<WebRTCPeer> CreatePeer( const std::string&)
{
#if USE_RE_STREAMER_2
    static std::unique_ptr<GstReStreamer2> reStreamer;

    if(!reStreamer) {
        reStreamer = std::make_unique<GstReStreamer2>("rtsp://ipcam.stream:8554/bars", "");
    }

    return reStreamer->createPeer();
#else
    return std::make_unique<GstReStreamer>("rtsp://ipcam.stream:8554/bars", "");
#endif

}

static std::unique_ptr<rtsp::ServerSession> CreateSession (
    const std::function<void (const rtsp::Request*)>& sendRequest,
    const std::function<void (const rtsp::Response*)>& sendResponse) noexcept
{
    return
        std::make_unique<ServerSession>(
            ServerSession::IceServers { "stun://stun.l.google.com:19302" },
            std::bind(CreatePeer, std::placeholders::_1),
            sendRequest,
            sendResponse);
}

int main(int argc, char *argv[])
{
    InitLwsLogger(spdlog::level::warn);

    LibGst libGst;

    http::Config httpConfig {
        .bindToLoopbackOnly = false
    };

    signalling::Config config {
        .bindToLoopbackOnly = false
    };

    GMainLoopPtr loopPtr(g_main_loop_new(nullptr, FALSE));
    GMainLoop* loop = loopPtr.get();

    lws_context_creation_info lwsInfo {};
    lwsInfo.gid = -1;
    lwsInfo.uid = -1;
    lwsInfo.options = LWS_SERVER_OPTION_EXPLICIT_VHOSTS;
#if defined(LWS_WITH_GLIB)
    lwsInfo.options |= LWS_SERVER_OPTION_GLIB;
    lwsInfo.foreign_loops = reinterpret_cast<void**>(&loop);
#endif

    LwsContextPtr contextPtr(lws_create_context(&lwsInfo));
    lws_context* context = contextPtr.get();

    std::string configJs =
        fmt::format("const WebRTSPPort = {};\r\n", config.port);
    http::Server httpServer(httpConfig, configJs, loop);
    signalling::WsServer server(
        config,
        loop,
        std::bind(
            CreateSession,
            std::placeholders::_1,
            std::placeholders::_2));

    if(httpServer.init(context) && server.init(context))
        g_main_loop_run(loop);
    else
        return -1;

    return 0;
}
