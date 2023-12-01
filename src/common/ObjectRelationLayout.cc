#include <includes/ObjectRelationLayout.h>

ObjectRelationLayout::ObjectRelationLayout(ImVec2 rootPosition, ImVec2 objectSize, ImVec2 objectSpacing)
    : m_rootPosition(rootPosition),
      m_objectSize(objectSize),
      m_objectSpacing(objectSpacing)
{
}

void ObjectRelationLayout::CalculateLevelGroups(ObjectInfoProxy *root, size_t depth)
{
    if (0 == depth)
        m_levelGroupInfo.push_back({});

    if (!root->childrens.empty())
    {
        if (m_levelGroupInfo.size() < depth + 2)
            m_levelGroupInfo.push_back({depth + 1});

        m_levelGroupInfo[depth + 1].groupsCount++;
        m_levelGroupInfo[depth + 1].groupParents.push_back(root);
    }

    for (auto &child : root->childrens)
        CalculateLevelGroups(child, depth + 1);
}

void ObjectRelationLayout::CalculateDefaultLayout(ObjectInfoProxy *root, size_t depth)
{
    static float currentY = 0;

    if (0 == depth)
    {
        root->position = m_rootPosition;
        currentY = m_rootPosition.y;
    }
    else
    {
        root->position.x = depth * m_objectSize.x + depth * m_objectSpacing.x;
        root->position.y = currentY;

        currentY += m_objectSize.y + m_objectSpacing.y;
    }

    for (auto &child : root->childrens)
        CalculateDefaultLayout(child, depth + 1);
}

void ObjectRelationLayout::CalculateCenterLayout(ObjectInfoProxy *root, size_t depth)
{
    if (0 == depth)
    {
        for (size_t i = 1; i < m_levelGroupInfo.size(); ++i)
        {
            auto &info = m_levelGroupInfo[i];

            auto parentNode = info.groupParents[info.groupsCount / 2];
            auto childNode = parentNode->childrens[parentNode->childrens.size() / 2];

            info.deltaY = parentNode->position.y - childNode->position.y;
            info.deltaY += m_levelGroupInfo[i - 1].deltaY;
        }
    }
    else
        root->position.y += m_levelGroupInfo[depth].deltaY;

    for (auto &child : root->childrens)
        CalculateCenterLayout(child, depth + 1);
}

void ObjectRelationLayout::MakeLayout(ObjectInfoProxy *root)
{
    m_levelGroupInfo.clear();

    CalculateDefaultLayout(root);

    CalculateLevelGroups(root);
    CalculateCenterLayout(root);
}