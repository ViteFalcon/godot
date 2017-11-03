#pragma once

#include <glm/glm.hpp>
#include <blib/Texture.h>
#include <blib/math/Rectangle.h>
#include <blib/TextureMap.h>
#include <blib/json.hpp>
#include <queue>

namespace blib
{
	class Texture;
	class Shader;
	class Font;
	class Renderer;
	class RenderState;

	class ResourceManager;
	class VertexP2T2C4C4;
	class VBO;


    class SpriteBatch
	{
	private:

        typedef VertexP2T2C4C4 vertexDef;


#define MAX_SPRITES 110000
		std::vector<vertexDef> vertices;
		std::vector<std::pair<const Texture*, unsigned short> > materialIndices;
		
		const Texture* currentTexture;
		Renderer* renderer;

		bool active;
		bool cacheActive;

        glm::mat4 matrix;

        size_t cacheStart;


	public:
		RenderState renderState;
		blib::Shader* shader;
		enum ShaderAttributes
		{
			ProjectionMatrix,
			Matrix,
			s_texture,
		};

		SpriteBatch(Renderer* renderer, ResourceManager* resourceManager, const RenderState &baseRenderState = RenderState());
		~SpriteBatch();

		virtual void begin(const glm::mat4 &matrix = glm::mat4(), FBO* fbo = NULL);
        virtual void end();

        inline const glm::mat4& getMatrix() const { return matrix; }
	};
}
