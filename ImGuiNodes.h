#ifndef IMGUI_NODES_H // !IMGUI_NODES_H
#define IMGUI_NODES_H

#define IMGUI_DEFINE_MATH_OPERATORS

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#include <vector>
#include <unordered_set>
#include <string_view>

namespace ImGui
{
    ////////////////////////////////////////////////////////////////////////////////

    enum ImGuiNodesConnectorType_
    {
        ImGuiNodesConnectorType_None = 0,
        ImGuiNodesConnectorType_Generic,
        ImGuiNodesConnectorType_Int,
        ImGuiNodesConnectorType_Float,
        ImGuiNodesConnectorType_Vector,
        ImGuiNodesConnectorType_Image,
        ImGuiNodesConnectorType_Text
    };

    enum ImGuiNodesNodeType_
    {
        ImGuiNodesNodeType_None = 0,
        ImGuiNodesNodeType_Generic,
        ImGuiNodesNodeType_Generator,
        ImGuiNodesNodeType_Test
    };

    enum ImGuiNodesConnectorStateFlag_
    {
        ImGuiNodesConnectorStateFlag_Default = 0,
        ImGuiNodesConnectorStateFlag_Visible = 1 << 0,
        ImGuiNodesConnectorStateFlag_Hovered = 1 << 1,
        ImGuiNodesConnectorStateFlag_Consider = 1 << 2,
        ImGuiNodesConnectorStateFlag_Draging = 1 << 3
    };

    enum ImGuiNodesNodeStateFlag_
    {
        ImGuiNodesNodeStateFlag_Default = 0,
        ImGuiNodesNodeStateFlag_Visible = 1 << 0,
        ImGuiNodesNodeStateFlag_Hovered = 1 << 1,
        ImGuiNodesNodeStateFlag_Marked = 1 << 2,
        ImGuiNodesNodeStateFlag_Selected = 1 << 3,
        ImGuiNodesNodeStateFlag_Collapsed = 1 << 4,
        ImGuiNodesNodeStateFlag_Disabled = 1 << 5,
        ImGuiNodesNodeStateFlag_Processing = 1 << 6
    };

    enum ImGuiNodesState_
    {
        ImGuiNodesState_Default = 0,
        ImGuiNodesState_HoveringNode,
        ImGuiNodesState_HoveringInput,
        ImGuiNodesState_HoveringOutput,
        ImGuiNodesState_Draging,
        ImGuiNodesState_DragingInput,
        ImGuiNodesState_DragingOutput,
        ImGuiNodesState_Selecting
    };

    ////////////////////////////////////////////////////////////////////////////////

    typedef unsigned int ImGuiNodesConnectorType;
    typedef unsigned int ImGuiNodesNodeType;

    typedef unsigned int ImGuiNodesConnectorState;
    typedef unsigned int ImGuiNodesNodeState;

    typedef unsigned int ImGuiNodesState;

    ////////////////////////////////////////////////////////////////////////////////

    // connector text name heights factors
    constexpr float ImGuiNodesConnectorDotDiameter = 0.7f; // connectors dot diameter
    constexpr float ImGuiNodesConnectorDotPadding = 0.35f; // connectors dot left/right sides padding
    constexpr float ImGuiNodesConnectorDistance = 1.5f;    // vertical distance between connectors centers

    // title text name heights factors
    constexpr float ImGuiNodesHSeparation = 1.7f; // extra horizontal separation inbetween IOs
    constexpr float ImGuiNodesVSeparation = 1.5f; // total IOs area separation from title and node bottom edge
    constexpr float ImGuiNodesTitleHight = 2.0f;

    struct ImGuiNodesNode;
    struct ImGuiNodesInput;
    struct ImGuiNodesOutput;

    ////////////////////////////////////////////////////////////////////////////////

    struct ImGuiNodesInput
    {
        ImVec2 pos_;
        ImRect area_input_;
        ImRect area_name_;
        ImGuiNodesConnectorType type_;
        ImGuiNodesConnectorState state_;
        const char *name_;
        ImGuiNodesNode *target_;
        ImGuiNodesOutput *output_;

        inline void TranslateInput(ImVec2 delta)
        {
            pos_ += delta;
            area_input_.Translate(delta);
            area_name_.Translate(delta);
        }

        inline void DrawInput(ImDrawList *draw_list, ImVec2 offset, float scale, ImGuiNodesState state) const
        {
            if (type_ == ImGuiNodesConnectorType_None)
                return;

            if (state != ImGuiNodesState_Draging && state_ & ImGuiNodesConnectorStateFlag_Hovered && false == (state_ & ImGuiNodesConnectorStateFlag_Consider))
            {
                const ImColor color = target_ == NULL ? ImColor(0.0f, 0.0f, 1.0f, 0.5f) : ImColor(1.0f, 0.5f, 0.0f, 0.5f);
                draw_list->AddRectFilled((area_input_.Min * scale) + offset, (area_input_.Max * scale) + offset, color);
            }

            if (state_ & (ImGuiNodesConnectorStateFlag_Consider | ImGuiNodesConnectorStateFlag_Draging))
                draw_list->AddRectFilled((area_input_.Min * scale) + offset, (area_input_.Max * scale) + offset, ImColor(0.0f, 1.0f, 0.0f, 0.5f));

            bool consider_fill = false;
            consider_fill |= bool(state_ & ImGuiNodesConnectorStateFlag_Draging);
            consider_fill |= bool(state_ & ImGuiNodesConnectorStateFlag_Hovered && state_ & ImGuiNodesConnectorStateFlag_Consider);

            ImColor color = consider_fill ? ImColor(0.0f, 1.0f, 0.0f, 1.0f) : ImColor(1.0f, 1.0f, 1.0f, 1.0f);

            consider_fill |= bool(target_);

            if (consider_fill)
                draw_list->AddCircleFilled((pos_ * scale) + offset, (ImGuiNodesConnectorDotDiameter * 0.5f) * area_name_.GetHeight() * scale, color);
            else
                draw_list->AddCircle((pos_ * scale) + offset, (ImGuiNodesConnectorDotDiameter * 0.5f) * area_name_.GetHeight() * scale, color);

            ImGui::SetCursorScreenPos((area_name_.Min * scale) + offset);
            ImGui::TextUnformatted(name_);
        }

        ImGuiNodesInput(const char *name, ImGuiNodesConnectorType type)
        {
            type_ = type;
            state_ = ImGuiNodesConnectorStateFlag_Default;
            target_ = NULL;
            output_ = NULL;
            name_ = name;

            area_name_.Min = ImVec2(0.0f, 0.0f);
            area_name_.Max = ImGui::CalcTextSize(name);

            area_input_.Min = ImVec2(0.0f, 0.0f);
            area_input_.Max.x = ImGuiNodesConnectorDotPadding + ImGuiNodesConnectorDotDiameter + ImGuiNodesConnectorDotPadding;
            area_input_.Max.y = ImGuiNodesConnectorDistance;
            area_input_.Max *= area_name_.GetHeight();

            ImVec2 offset = ImVec2(0.0f, 0.0f) - area_input_.GetCenter();

            area_name_.Translate(ImVec2(area_input_.GetWidth(), (area_input_.GetHeight() - area_name_.GetHeight()) * 0.5f));

            area_input_.Max.x += area_name_.GetWidth();
            area_input_.Max.x += ImGuiNodesConnectorDotPadding * area_name_.GetHeight();

            area_input_.Translate(offset);
            area_name_.Translate(offset);
        }
    };

    struct ImGuiNodesOutput
    {
        ImVec2 pos_;
        ImRect area_output_;
        ImRect area_name_;
        ImGuiNodesConnectorType type_;
        ImGuiNodesConnectorState state_;
        const char *name_;
        unsigned int connections_;

        inline void TranslateOutput(ImVec2 delta)
        {
            pos_ += delta;
            area_output_.Translate(delta);
            area_name_.Translate(delta);
        }

        inline void DrawOutput(ImDrawList *draw_list, ImVec2 offset, float scale, ImGuiNodesState state) const
        {
            if (type_ == ImGuiNodesConnectorType_None)
                return;

            if (state != ImGuiNodesState_Draging && state_ & ImGuiNodesConnectorStateFlag_Hovered && false == (state_ & ImGuiNodesConnectorStateFlag_Consider))
                draw_list->AddRectFilled((area_output_.Min * scale) + offset, (area_output_.Max * scale) + offset, ImColor(0.0f, 0.0f, 1.0f, 0.5f));

            if (state_ & (ImGuiNodesConnectorStateFlag_Consider | ImGuiNodesConnectorStateFlag_Draging))
                draw_list->AddRectFilled((area_output_.Min * scale) + offset, (area_output_.Max * scale) + offset, ImColor(0.0f, 1.0f, 0.0f, 0.5f));

            bool consider_fill = false;
            consider_fill |= bool(state_ & ImGuiNodesConnectorStateFlag_Draging);
            consider_fill |= bool(state_ & ImGuiNodesConnectorStateFlag_Hovered && state_ & ImGuiNodesConnectorStateFlag_Consider);

            ImColor color = consider_fill ? ImColor(0.0f, 1.0f, 0.0f, 1.0f) : ImColor(1.0f, 1.0f, 1.0f, 1.0f);

            consider_fill |= bool(connections_ > 0);

            if (consider_fill)
                draw_list->AddCircleFilled((pos_ * scale) + offset, (ImGuiNodesConnectorDotDiameter * 0.5f) * area_name_.GetHeight() * scale, color);
            else
                draw_list->AddCircle((pos_ * scale) + offset, (ImGuiNodesConnectorDotDiameter * 0.5f) * area_name_.GetHeight() * scale, color);

            ImGui::SetCursorScreenPos((area_name_.Min * scale) + offset);
            ImGui::TextUnformatted(name_);
        }

        ImGuiNodesOutput(const char *name, ImGuiNodesConnectorType type)
        {
            type_ = type;
            state_ = ImGuiNodesConnectorStateFlag_Default;
            connections_ = 0;
            name_ = name;

            area_name_.Min = ImVec2(0.0f, 0.0f) - ImGui::CalcTextSize(name);
            area_name_.Max = ImVec2(0.0f, 0.0f);

            area_output_.Min.x = ImGuiNodesConnectorDotPadding + ImGuiNodesConnectorDotDiameter + ImGuiNodesConnectorDotPadding;
            area_output_.Min.y = ImGuiNodesConnectorDistance;
            area_output_.Min *= -area_name_.GetHeight();
            area_output_.Max = ImVec2(0.0f, 0.0f);

            ImVec2 offset = ImVec2(0.0f, 0.0f) - area_output_.GetCenter();

            area_name_.Translate(ImVec2(area_output_.Min.x, (area_output_.GetHeight() - area_name_.GetHeight()) * -0.5f));

            area_output_.Min.x -= area_name_.GetWidth();
            area_output_.Min.x -= ImGuiNodesConnectorDotPadding * area_name_.GetHeight();

            area_output_.Translate(offset);
            area_name_.Translate(offset);
        }
    };

    struct ImGuiNodesNode
    {
        ImRect area_node_;
        ImRect area_name_;
        float title_height_;
        float body_height_;
        ImGuiNodesNodeState state_;
        ImGuiNodesNodeType type_;
        const char *name_;
        ImColor color_;
        std::vector<ImGuiNodesInput> inputs_;
        std::vector<ImGuiNodesOutput> outputs_;

        inline void TranslateNode(ImVec2 delta, bool selected_only = false)
        {
            if (selected_only && !(state_ & ImGuiNodesNodeStateFlag_Selected))
                return;

            area_node_.Translate(delta);
            area_name_.Translate(delta);

            for (int input_idx = 0; input_idx < inputs_.size(); ++input_idx)
                inputs_[input_idx].TranslateInput(delta);

            for (int output_idx = 0; output_idx < outputs_.size(); ++output_idx)
                outputs_[output_idx].TranslateOutput(delta);
        }

        inline void BuildNodeGeometry(ImVec2 inputs_size, ImVec2 outputs_size)
        {
            body_height_ = ImMax(inputs_size.y, outputs_size.y) + (ImGuiNodesVSeparation * area_name_.GetHeight());

            area_node_.Min = ImVec2(0.0f, 0.0f);
            area_node_.Max = ImVec2(0.0f, 0.0f);
            area_node_.Max.x += inputs_size.x + outputs_size.x;
            area_node_.Max.x += ImGuiNodesHSeparation * area_name_.GetHeight();
            area_node_.Max.y += title_height_ + body_height_;

            area_name_.Translate(ImVec2((area_node_.GetWidth() - area_name_.GetWidth()) * 0.5f, ((title_height_ - area_name_.GetHeight()) * 0.5f)));

            ImVec2 inputs = area_node_.GetTL();
            inputs.y += title_height_ + (ImGuiNodesVSeparation * area_name_.GetHeight() * 0.5f);
            for (int input_idx = 0; input_idx < inputs_.size(); ++input_idx)
            {
                inputs_[input_idx].TranslateInput(inputs - inputs_[input_idx].area_input_.GetTL());
                inputs.y += inputs_[input_idx].area_input_.GetHeight();
            }

            ImVec2 outputs = area_node_.GetTR();
            outputs.y += title_height_ + (ImGuiNodesVSeparation * area_name_.GetHeight() * 0.5f);
            for (int output_idx = 0; output_idx < outputs_.size(); ++output_idx)
            {
                outputs_[output_idx].TranslateOutput(outputs - outputs_[output_idx].area_output_.GetTR());
                outputs.y += outputs_[output_idx].area_output_.GetHeight();
            }
        }

        inline void DrawNode(ImDrawList *draw_list, ImVec2 offset, float scale, ImGuiNodesState state) const
        {
            if (false == (state_ & ImGuiNodesNodeStateFlag_Visible))
                return;

            ImRect node_rect = area_node_;
            node_rect.Min *= scale;
            node_rect.Max *= scale;
            node_rect.Translate(offset);

            float rounding = title_height_ * scale * 0.3f;

            ImColor head_color = color_, body_color = color_;
            head_color.Value.x *= 0.5;
            head_color.Value.y *= 0.5;
            head_color.Value.z *= 0.5;

            head_color.Value.w = 1.00f;
            body_color.Value.w = 0.75f;

            const ImVec2 outline(4.0f * scale, 4.0f * scale);

            const ImDrawFlags rounding_corners_flags = ImDrawCornerFlags_All;

            if (state_ & ImGuiNodesNodeStateFlag_Disabled)
            {
                body_color.Value.w = 0.25f;

                if (state_ & ImGuiNodesNodeStateFlag_Collapsed)
                    head_color.Value.w = 0.25f;
            }

            if (state_ & ImGuiNodesNodeStateFlag_Processing)
                draw_list->AddRectFilled(node_rect.Min - outline, node_rect.Max + outline, body_color, rounding, rounding_corners_flags);
            else
                draw_list->AddRectFilled(node_rect.Min, node_rect.Max, body_color, rounding, rounding_corners_flags);

            const ImVec2 head = node_rect.GetTR() + ImVec2(0.0f, title_height_ * scale);

            if (false == (state_ & ImGuiNodesNodeStateFlag_Collapsed))
                draw_list->AddLine(ImVec2(node_rect.Min.x, head.y), ImVec2(head.x - 1.0f, head.y), ImColor(0.0f, 0.0f, 0.0f, 0.5f), 2.0f);

            const ImDrawFlags head_corners_flags = state_ & ImGuiNodesNodeStateFlag_Collapsed ? rounding_corners_flags : ImDrawCornerFlags_Top;
            draw_list->AddRectFilled(node_rect.Min, head, head_color, rounding, head_corners_flags);

            ////////////////////////////////////////////////////////////////////////////////

            if (state_ & ImGuiNodesNodeStateFlag_Disabled)
            {
                IM_ASSERT(false == node_rect.IsInverted());

                const float separation = 15.0f * scale;

                for (float line = separation; true; line += separation)
                {
                    ImVec2 start = node_rect.Min + ImVec2(0.0f, line);
                    ImVec2 stop = node_rect.Min + ImVec2(line, 0.0f);

                    if (start.y > node_rect.Max.y)
                        start = ImVec2(start.x + (start.y - node_rect.Max.y), node_rect.Max.y);

                    if (stop.x > node_rect.Max.x)
                        stop = ImVec2(node_rect.Max.x, stop.y + (stop.x - node_rect.Max.x));

                    if (start.x > node_rect.Max.x)
                        break;

                    if (stop.y > node_rect.Max.y)
                        break;

                    draw_list->AddLine(start, stop, body_color, 3.0f * scale);
                }
            }

            ////////////////////////////////////////////////////////////////////////////////

            if (false == (state_ & ImGuiNodesNodeStateFlag_Collapsed))
            {
                for (int input_idx = 0; input_idx < inputs_.size(); ++input_idx)
                    inputs_[input_idx].DrawInput(draw_list, offset, scale, state);

                for (int output_idx = 0; output_idx < outputs_.size(); ++output_idx)
                    outputs_[output_idx].DrawOutput(draw_list, offset, scale, state);
            }

            ////////////////////////////////////////////////////////////////////////////////

            ImGui::SetCursorScreenPos(((area_name_.Min + ImVec2(2, 2)) * scale) + offset);
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 0, 0, 255));
            ImGui::TextUnformatted(name_);
            ImGui::PopStyleColor();

            ImGui::SetCursorScreenPos((area_name_.Min * scale) + offset);
            ImGui::TextUnformatted(name_);

            if (state_ & (ImGuiNodesNodeStateFlag_Marked | ImGuiNodesNodeStateFlag_Selected))
                draw_list->AddRectFilled(node_rect.Min, node_rect.Max, ImColor(1.0f, 1.0f, 1.0f, 0.25f), rounding, rounding_corners_flags);

            if (state_ & ImGuiNodesNodeStateFlag_Processing)
            {
                ImColor processing_color = color_;
                processing_color.Value.x *= 1.5;
                processing_color.Value.y *= 1.5;
                processing_color.Value.z *= 1.5;
                processing_color.Value.w = 1.0f;

                draw_list->AddRect(node_rect.Min - outline, node_rect.Max + outline, processing_color, rounding, rounding_corners_flags, 2.0f * scale);
            }
            else
            {
                draw_list->AddRect(
                    node_rect.Min - outline * 0.5f,
                    node_rect.Max + outline * 0.5f,
                    ImColor(0.0f, 0.0f, 0.0f, 0.5f),
                    rounding,
                    rounding_corners_flags,
                    3.0f * scale);
            }
        }

        ImGuiNodesNode(const char *name, ImGuiNodesNodeType type, ImColor color)
        {
            name_ = name;
            type_ = type;
            state_ = ImGuiNodesNodeStateFlag_Default;
            color_ = color;

            area_name_.Min = ImVec2(0.0f, 0.0f);
            area_name_.Max = ImGui::CalcTextSize(name);
            title_height_ = ImGuiNodesTitleHight * area_name_.GetHeight();
        }
    };

    ////////////////////////////////////////////////////////////////////////////////

    // ImGuiNodesConnectionDesc size round up to 32 bytes to be cache boundaries friendly
    constexpr int ImGuiNodesNamesMaxLen = 32 - sizeof(ImGuiNodesConnectorType);

    struct ImGuiNodesConnectionDesc
    {
        char name_[ImGuiNodesNamesMaxLen];
        ImGuiNodesConnectorType type_;
    };

    struct ImGuiNodesNodeDesc
    {
        char name_[ImGuiNodesNamesMaxLen];
        ImGuiNodesNodeType type_;
        ImColor color_;
        std::vector<ImGuiNodesConnectionDesc> inputs_;
        std::vector<ImGuiNodesConnectionDesc> outputs_;

        struct Hash
        {
            std::size_t operator()(const ImGuiNodesNodeDesc &node) const
            {
                return std::hash<std::string_view>{}(node.name_);
            }
        };

        struct Equal
        {
            bool operator()(const ImGuiNodesNodeDesc &lhs, const ImGuiNodesNodeDesc &rhs) const
            {
                return std::string_view{lhs.name_} == std::string_view{rhs.name_};
            }
        };
    };

    ////////////////////////////////////////////////////////////////////////////////

    struct ImGuiNodes
    {
    private:
        ImVec2 mouse_;
        ImVec2 pos_;
        ImVec2 size_;
        ImVec2 scroll_;
        ImVec4 connection_;
        float scale_;

        ////////////////////////////////////////////////////////////////////////////////

        ImGuiNodesState state_;

        ImRect area_;
        ImGuiNodesNode *element_node_ = NULL;
        ImGuiNodesInput *element_input_ = NULL;
        ImGuiNodesOutput *element_output_ = NULL;
        ImGuiNodesNode *processing_node_ = NULL;

        ////////////////////////////////////////////////////////////////////////////////

        std::vector<ImGuiNodesNode *> nodes_;
        std::unordered_set<ImGuiNodesNodeDesc, ImGuiNodesNodeDesc::Hash, ImGuiNodesNodeDesc::Equal> nodes_desc_;

        ////////////////////////////////////////////////////////////////////////////////

    private:
        void UpdateCanvasGeometry(ImDrawList *draw_list);
        ImGuiNodesNode *UpdateNodesFromCanvas();
        ImGuiNodesNode *CreateNodeFromDesc(ImGuiNodesNodeDesc *desc, ImVec2 pos);

        inline void DrawConnection(ImVec2 p1, ImVec2 p4, ImColor color)
        {
            ImDrawList *draw_list = ImGui::GetWindowDrawList();

            float line = 25.0f;

            ImVec2 p2 = p1;
            ImVec2 p3 = p4;

            p2 += (ImVec2(-line, 0.0f) * scale_);
            p3 += (ImVec2(+line, 0.0f) * scale_);

            draw_list->AddBezierCubic(p1, p2, p3, p4, color, 1.5f * scale_);
        }

        inline bool ConnectionMatrix(ImGuiNodesNode *input_node, ImGuiNodesNode *output_node, ImGuiNodesInput *input, ImGuiNodesOutput *output)
        {
            if (input->target_ && input->target_ == output_node)
                return false;

            if (input->type_ == output->type_)
                return true;

            if (input->type_ == ImGuiNodesConnectorType_Generic)
                return true;

            if (output->type_ == ImGuiNodesConnectorType_Generic)
                return true;

            return false;
        }

        inline bool SortSelectedNodesOrder();

    public:
        void Update();
        void ProcessNodes();
        void ProcessContextMenu();

        ImGuiNodes(bool add_default_node_desc = false)
        {
            scale_ = 1.0f;
            state_ = ImGuiNodesState_Default;
            element_node_ = NULL;
            element_input_ = NULL;
            element_output_ = NULL;

            ////////////////////////////////////////////////////////////////////////////////

            if (add_default_node_desc)
            {
                nodes_desc_.insert(
                    {
                        .name_ = "Test",
                        .type_ = ImGuiNodesNodeType_Generic,
                        .color_ = ImColor(0.2f, 0.3f, 0.6f, 0.0f),
                        .inputs_ = {
                            {"Float", ImGuiNodesConnectorType_Float},
                            {"Int", ImGuiNodesConnectorType_Int},
                            {"TextStream", ImGuiNodesConnectorType_Text},
                        },
                        .outputs_ = {
                            {"Float", ImGuiNodesConnectorType_Float},
                        },
                    });

                nodes_desc_.insert(
                    {
                        .name_ = "InputBox",
                        .type_ = ImGuiNodesNodeType_Generic,
                        .color_ = ImColor(0.3f, 0.5f, 0.5f, 0.0f),
                        .inputs_ = {
                            {"Float1", ImGuiNodesConnectorType_Float},
                            {"Float2", ImGuiNodesConnectorType_Float},
                            {"Int1", ImGuiNodesConnectorType_Int},
                            {"Int2", ImGuiNodesConnectorType_Int},
                            {"", ImGuiNodesConnectorType_None},
                            {"GenericSink", ImGuiNodesConnectorType_Generic},
                            {"", ImGuiNodesConnectorType_None},
                            {"Vector", ImGuiNodesConnectorType_Vector},
                            {"Image", ImGuiNodesConnectorType_Image},
                            {"Text", ImGuiNodesConnectorType_Text},
                        },
                        .outputs_ = {
                            {"TextStream", ImGuiNodesConnectorType_Text},
                            {"", ImGuiNodesConnectorType_None},
                            {"Float", ImGuiNodesConnectorType_Float},
                            {"", ImGuiNodesConnectorType_None},
                            {"Int", ImGuiNodesConnectorType_Int},
                        },
                    });

                nodes_desc_.insert(
                    {
                        .name_ = "OutputBox",
                        .type_ = ImGuiNodesNodeType_Generic,
                        .color_ = ImColor(0.4f, 0.3f, 0.5f, 0.0f),
                        .inputs_ = {
                            {"GenericSink1", ImGuiNodesConnectorType_Generic},
                            {"GenericSink2", ImGuiNodesConnectorType_Generic},
                            {"", ImGuiNodesConnectorType_None},
                            {"Float", ImGuiNodesConnectorType_Float},
                            {"Int", ImGuiNodesConnectorType_Int},
                            {"Text", ImGuiNodesConnectorType_Text},
                        },
                        .outputs_ = {
                            {"Vector", ImGuiNodesConnectorType_Vector},
                            {"Image", ImGuiNodesConnectorType_Image},
                            {"Text", ImGuiNodesConnectorType_Text},
                            {"", ImGuiNodesConnectorType_None},
                            {"Float", ImGuiNodesConnectorType_Float},
                            {"Int", ImGuiNodesConnectorType_Int},
                            {"", ImGuiNodesConnectorType_None},
                            {"", ImGuiNodesConnectorType_None},
                            {"", ImGuiNodesConnectorType_None},
                            {"Generic", ImGuiNodesConnectorType_Generic},
                        },
                    });
            }

            ////////////////////////////////////////////////////////////////////////////////

            return;
        }

        ~ImGuiNodes()
        {
            for (int node_idx = 0; node_idx < nodes_.size(); ++node_idx)
                delete nodes_[node_idx];
        }
    };

    ////////////////////////////////////////////////////////////////////////////////
}

#undef IMGUI_DEFINE_MATH_OPERATORS

#endif // !IMGUI_NODES_H