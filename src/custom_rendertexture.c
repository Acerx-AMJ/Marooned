#include "custom_rendertexture.h"
#include "raylib.h"
#include "rlgl.h"
#include <stddef.h>

// Load texture for rendering (framebuffer) with DEPTH as a TEXTURE (usable in shader)
RenderTexture2D LoadRenderTextureWithDepthTexture(int width, int height)
{
    RenderTexture2D target = { 0 };

    target.id = rlLoadFramebuffer(); // Load an empty framebuffer

    if (target.id > 0)
    {
        rlEnableFramebuffer(target.id);

        // Create color texture (RGBA)
        target.texture.id = rlLoadTexture(NULL, width, height, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8, 1);
        target.texture.width = width;
        target.texture.height = height;
        target.texture.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
        target.texture.mipmaps = 1;

        // Create depth TEXTURE instead of RenderBuffer
        target.depth.id = rlLoadTextureDepth(width, height, false);   // <<== FALSE HERE
        target.depth.width = width;
        target.depth.height = height;
        target.depth.format = 19;       // PIXELFORMAT_DEPTH_COMPONENT_24BIT (raylib doesn't define it well)
        target.depth.mipmaps = 1;

        // Attach color texture and depth TEXTURE to FBO
        rlFramebufferAttach(target.id, target.texture.id, RL_ATTACHMENT_COLOR_CHANNEL0, RL_ATTACHMENT_TEXTURE2D, 0);
        rlFramebufferAttach(target.id, target.depth.id, RL_ATTACHMENT_DEPTH, RL_ATTACHMENT_TEXTURE2D, 0);



        // Check FBO completeness
        if (rlFramebufferComplete(target.id))
            TRACELOG(LOG_INFO, "FBO: [ID %i] Framebuffer with depth texture created successfully", target.id);
        else
            TRACELOG(LOG_WARNING, "FBO: [ID %i] Framebuffer creation failed", target.id);

        rlDisableFramebuffer();
    }
    else
    {
        TRACELOG(LOG_WARNING, "FBO: Framebuffer object can not be created");
    }

    return target;
}
