#ifndef SOFTWARERENDERENGINE_H_
#define SOFTWARERENDERENGINE_H_

#include <core/rect.h>
#include <core/image.h>

#include <game/outputmode.h>
#include <game/sdl_utils.h>
#include <game/renderengine.h>

namespace core
{
    class Size;
    class Line;
    class Point;
}

namespace castle
{
    namespace render
    {
        class SoftwareRenderEngine : public RenderEngine
        {
            RendererPtr mScreenRenderer;
            WindowPtr mWindow;

            /** mOutputMode is not yet active output mode but pending out
                and mFrameOutputMode is current frame's mode
            **/
            OutputMode mOutputMode;
            OutputMode mFrameOutputMode;

            core::Rect mViewport;
            core::Image mScreenImage;
            TexturePtr mScreenTexture;
            RendererPtr mPrimitiveRenderer;
            int mOpacityMod;
        
        public:
            explicit SoftwareRenderEngine();
            virtual ~SoftwareRenderEngine();
        
            virtual void BeginFrame();
            virtual void EndFrame();
            virtual void SetOutputMode(const OutputMode &mode);
            virtual const OutputMode GetOutputMode() const;
            virtual const core::Size GetMaxOutputSize() const;
            virtual void DrawPoints(const core::Point *points, size_t count, const core::Color &color);
            virtual void DrawRects(const core::Rect *rects, size_t count, const core::Color &color, DrawMode mode);
            virtual void DrawLines(const core::Line *lines, size_t count, const core::Color &color);
            virtual void DrawPolygon(const core::Point *points, size_t count, const core::Color &color, DrawMode mode);
            virtual void DrawImage(const core::Image &image, const core::Rect &subrect, const core::Point &targetPoint);
            virtual void DrawImageTiled(const core::Image &image, const core::Rect &source, const core::Rect &target);
            virtual void DrawImageScaled(const core::Image &image, const core::Rect &source, const core::Rect &target);
            virtual void SetOpacityMod(int opacity);
            virtual void SetViewport(const core::Rect &rect);
            virtual void ClearOutput(const core::Color &color);

        private:
            bool ReallocationRequired(const OutputMode &mode) const;
            void UpdateDrawColor(const core::Color &color);
            void UpdateViewport(const core::Rect &clipRect);
        };
    }
}

#endif // SOFTWARERENDERENGINE_H_
