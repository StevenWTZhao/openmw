#ifndef COMPONENTS_TERRAIN_H
#define COMPONENTS_TERRAIN_H

#include <OgreHardwareIndexBuffer.h>
#include <OgreHardwareVertexBuffer.h>
#include <OgreAxisAlignedBox.h>
#include <OgreTexture.h>

namespace Ogre
{
    class Camera;
}

namespace Terrain
{

    class QuadTreeNode;
    class Storage;

    /**
     * @brief A quadtree-based terrain implementation suitable for large data sets. \n
     *        Near cells are rendered with alpha splatting, distant cells are merged
     *        together in batches and have their layers pre-rendered onto a composite map. \n
     *        Cracks at LOD transitions are avoided using stitching.
     * @note  Multiple cameras are not supported yet
     */
    class Terrain
    {
    public:
        /// @note takes ownership of \a storage
        Terrain(Ogre::SceneManager* sceneMgr, Storage* storage, int visiblityFlags);
        ~Terrain();

        /// Update chunk LODs according to this camera position
        /// @note Calling this method might lead to composite textures being rendered, so it is best
        /// not to call it when render commands are still queued, since that would cause a flush.
        void update (Ogre::Camera* camera);

        /// \todo
        float getHeightAt (const Ogre::Vector3& worldPos) { return 0; }

        /// Get the world bounding box of a chunk of terrain centered at \a center
        Ogre::AxisAlignedBox getWorldBoundingBox (const Ogre::Vector2& center);

        Ogre::SceneManager* getSceneManager() { return mSceneMgr; }

        Storage* getStorage() { return mStorage; }

        /// Show or hide the whole terrain
        void setVisible(bool visible);

        /// Recreate materials used by terrain chunks. This should be called whenever settings of
        /// the material factory are changed. (Relying on the factory to update those materials is not
        /// enough, since turning a feature on/off can change the number of texture units available for layer/blend
        /// textures, and to properly respond to this we may need to change the structure of the material, such as
        /// adding or removing passes. This can only be achieved by a full rebuild.)
        void applyMaterials();

        int getVisiblityFlags() { return mVisibilityFlags; }

        int getMaxBatchSize() { return mMaxBatchSize; }

        void enableSplattingShader(bool enabled);

    private:
        QuadTreeNode* mRootNode;
        Storage* mStorage;

        int mVisibilityFlags;

        Ogre::SceneManager* mSceneMgr;
        Ogre::SceneManager* mCompositeMapSceneMgr;

        /// Bounds in cell units
        Ogre::AxisAlignedBox mBounds;

        /// Minimum size of a terrain batch along one side (in cell units)
        float mMinBatchSize;
        /// Maximum size of a terrain batch along one side (in cell units)
        float mMaxBatchSize;

        void buildQuadTree(QuadTreeNode* node);

    public:
        // ----INTERNAL----

        enum IndexBufferFlags
        {
            IBF_North = 1 << 0,
            IBF_East  = 1 << 1,
            IBF_South = 1 << 2,
            IBF_West  = 1 << 3
        };

        /// @param flags first 4*4 bits are LOD deltas on each edge, respectively (4 bits each)
        ///              next 4 bits are LOD level of the index buffer (LOD 0 = don't omit any vertices)
        /// @param numIndices number of indices that were used will be written here
        Ogre::HardwareIndexBufferSharedPtr getIndexBuffer (int flags, size_t& numIndices);

        Ogre::HardwareVertexBufferSharedPtr getVertexBuffer (int numVertsOneSide);

        Ogre::SceneManager* getCompositeMapSceneManager() { return mCompositeMapSceneMgr; }

        // Delete all quads
        void clearCompositeMapSceneManager();
        void renderCompositeMap (Ogre::TexturePtr target);

    private:
        // Index buffers are shared across terrain batches where possible. There is one index buffer for each
        // combination of LOD deltas and index buffer LOD we may need.
        std::map<int, Ogre::HardwareIndexBufferSharedPtr> mIndexBufferMap;

        std::map<int, Ogre::HardwareVertexBufferSharedPtr> mUvBufferMap;

        Ogre::RenderTarget* mCompositeMapRenderTarget;
        Ogre::TexturePtr mCompositeMapRenderTexture;
    };

}

#endif
