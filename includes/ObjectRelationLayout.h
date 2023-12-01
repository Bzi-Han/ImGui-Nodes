#ifndef OBJECT_RELATION_LAYOUT_H // !OBJECT_RELATION_LAYOUT_H
#define OBJECT_RELATION_LAYOUT_H

#include <imgui/imgui.h>

#include <vector>

class ObjectRelationLayout
{
public:
    struct ObjectInfoProxy
    {
        ImVec2 position;
        std::vector<ObjectInfoProxy *> childrens;
    };

public:
    void MakeLayout(ObjectInfoProxy *root);

public:
    ObjectRelationLayout(ImVec2 rootPosition, ImVec2 objectSize, ImVec2 objectSpacing);
    ObjectRelationLayout(ImVec2 rootPosition) : ObjectRelationLayout(rootPosition, {300.f, 50.f}, {100.f, 20.f}) {}
    ObjectRelationLayout() : ObjectRelationLayout({0.f, 0.f}) {}
    virtual ~ObjectRelationLayout() {}

private:
    void CalculateLevelGroups(ObjectInfoProxy *root, size_t depth = 0);

    void CalculateDefaultLayout(ObjectInfoProxy *root, size_t depth = 0);
    void CalculateCenterLayout(ObjectInfoProxy *root, size_t depth = 0);

private:
    struct Level
    {
        size_t level;
        size_t groupsCount;
        std::vector<ObjectInfoProxy *> groupParents;

        float deltaY;
    };

    ImVec2 m_rootPosition;
    ImVec2 m_objectSize;
    ImVec2 m_objectSpacing;

    std::vector<Level> m_levelGroupInfo;
};

#endif // !OBJECT_RELATION_LAYOUT_H