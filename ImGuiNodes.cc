#include "ImGuiNodes.h"

#include <cmath>

namespace ImGui
{
    void ImGuiNodes::UpdateCanvasGeometry(ImDrawList *draw_list)
    {
        const ImGuiIO &io = ImGui::GetIO();

        mouse_ = ImGui::GetMousePos();

        {
            ImVec2 min = ImGui::GetWindowContentRegionMin();
            ImVec2 max = ImGui::GetWindowContentRegionMax();

            pos_ = ImGui::GetWindowPos() + min;
            size_ = max - min;
        }

        ImRect canvas(pos_, pos_ + size_);

        ////////////////////////////////////////////////////////////////////////////////

        if (ImGui::IsKeyPressed(ImGuiKey_Home))
        {
            scroll_ = {};
            scale_ = 1.0f;
        }

        ////////////////////////////////////////////////////////////////////////////////

        if (false == ImGui::IsMouseDown(ImGuiMouseButton_Left) && canvas.Contains(mouse_))
        {
            if (ImGui::IsMouseDragging(ImGuiMouseButton_Middle))
                scroll_ += io.MouseDelta;

            if (io.KeyShift && !io.KeyCtrl)
                scroll_.x += io.MouseWheel * 16.0f;

            if (!io.KeyShift && !io.KeyCtrl)
                scroll_.y += io.MouseWheel * 16.0f;

            if (!io.KeyShift && io.KeyCtrl)
            {
                ImVec2 focus = (mouse_ - scroll_ - pos_) / scale_;

                if (io.MouseWheel < 0.0f)
                    for (float zoom = io.MouseWheel; zoom < 0.0f; zoom += 1.0f)
                        scale_ = ImMax(0.3f, scale_ / 1.05f);

                if (io.MouseWheel > 0.0f)
                    for (float zoom = io.MouseWheel; zoom > 0.0f; zoom -= 1.0f)
                        scale_ = ImMin(3.0f, scale_ * 1.05f);

                ImVec2 shift = scroll_ + (focus * scale_);
                scroll_ += mouse_ - shift - pos_;
            }

            if (ImGui::IsMouseReleased(ImGuiMouseButton_Right) && element_node_ == NULL)
                if (io.MouseDragMaxDistanceSqr[ImGuiMouseButton_Right] < (io.MouseDragThreshold * io.MouseDragThreshold))
                {
                    bool selected = false;

                    for (int node_idx = 0; node_idx < nodes_.size(); ++node_idx)
                    {
                        if (nodes_[node_idx]->state_ & ImGuiNodesNodeStateFlag_Selected)
                        {
                            selected = true;
                            break;
                        }
                    }

                    if (false == selected && !nodes_desc_.empty())
                        ImGui::OpenPopup("NodesContextMenu");
                }
        }

        ////////////////////////////////////////////////////////////////////////////////

        const float grid = 64.0f * scale_;

        int mark_x = (int)(scroll_.x / grid);
        for (float x = (std::fmodf)(scroll_.x, grid); x < size_.x; x += grid, --mark_x)
        {
            if (0.f > x)
                continue;

            ImColor color = mark_x % 5 ? ImColor(0.5f, 0.5f, 0.5f, 0.1f) : ImColor(1.0f, 1.0f, 1.0f, 0.1f);
            draw_list->AddLine(ImVec2(x, 0.0f) + pos_, ImVec2(x, size_.y) + pos_, color, 0.1f);
        }

        int mark_y = (int)(scroll_.y / grid);
        for (float y = (std::fmodf)(scroll_.y, grid); y < size_.y; y += grid, --mark_y)
        {
            if (0.f > y)
                continue;

            ImColor color = mark_y % 5 ? ImColor(0.5f, 0.5f, 0.5f, 0.1f) : ImColor(1.0f, 1.0f, 1.0f, 0.1f);
            draw_list->AddLine(ImVec2(0.0f, y) + pos_, ImVec2(size_.x, y) + pos_, color, 0.1f);
        }

        draw_list->AddRect(ImGui::GetWindowContentRegionMin(), ImGui::GetWindowContentRegionMax(), ImColor(0.247f, 0.247f, 0.282f, 1.000f));
    }

    ImGuiNodesNode *ImGuiNodes::UpdateNodesFromCanvas()
    {
        if (nodes_.empty())
            return NULL;

        const ImGuiIO &io = ImGui::GetIO();

        ImVec2 offset = pos_ + scroll_;
        ImRect canvas(pos_, pos_ + size_);
        ImGuiNodesNode *hovered_node = NULL;

        for (size_t node_idx = nodes_.size(); node_idx != 0;)
        {
            ImGuiNodesNode *node = nodes_[--node_idx];
            IM_ASSERT(node);

            ImRect node_rect = node->area_node_;
            node_rect.Min *= scale_;
            node_rect.Max *= scale_;
            node_rect.Translate(offset);

            node_rect.ClipWith(canvas);

            if (canvas.Overlaps(node_rect))
            {
                node->state_ |= ImGuiNodesNodeStateFlag_Visible;
                node->state_ &= ~ImGuiNodesNodeStateFlag_Hovered;
            }
            else
            {
                node->state_ &= ~(ImGuiNodesNodeStateFlag_Visible | ImGuiNodesNodeStateFlag_Hovered | ImGuiNodesNodeStateFlag_Marked);
                continue;
            }

            if (NULL == hovered_node && node_rect.Contains(mouse_))
                hovered_node = node;

            ////////////////////////////////////////////////////////////////////////////////

            if (state_ == ImGuiNodesState_Selecting)
            {
                if (io.KeyCtrl && area_.Overlaps(node_rect))
                {
                    node->state_ |= ImGuiNodesNodeStateFlag_Marked;
                    continue;
                }

                if (false == io.KeyCtrl && area_.Contains(node_rect))
                {
                    node->state_ |= ImGuiNodesNodeStateFlag_Marked;

                    continue;
                }

                node->state_ &= ~ImGuiNodesNodeStateFlag_Marked;
            }

            ////////////////////////////////////////////////////////////////////////////////

            for (int input_idx = 0; input_idx < node->inputs_.size(); ++input_idx)
            {
                ImGuiNodesInput &input = node->inputs_[input_idx];

                if (input.type_ == ImGuiNodesConnectorType_None)
                    continue;

                input.state_ &= ~(ImGuiNodesConnectorStateFlag_Hovered | ImGuiNodesConnectorStateFlag_Consider | ImGuiNodesConnectorStateFlag_Draging);

                if (state_ == ImGuiNodesState_DragingInput)
                {
                    if (&input == element_input_)
                        input.state_ |= ImGuiNodesConnectorStateFlag_Draging;

                    continue;
                }

                if (state_ == ImGuiNodesState_DragingOutput)
                {
                    if (element_node_ == node)
                        continue;

                    if (ConnectionMatrix(node, element_node_, &input, element_output_))
                        input.state_ |= ImGuiNodesConnectorStateFlag_Consider;
                }

                if (!hovered_node || hovered_node != node)
                    continue;

                if (state_ == ImGuiNodesState_Selecting)
                    continue;

                if (state_ != ImGuiNodesState_DragingOutput && node->state_ & ImGuiNodesNodeStateFlag_Selected)
                    continue;

                ImRect input_rect = input.area_input_;
                input_rect.Min *= scale_;
                input_rect.Max *= scale_;
                input_rect.Translate(offset);

                if (input_rect.Contains(mouse_))
                {
                    if (state_ != ImGuiNodesState_DragingOutput)
                    {
                        input.state_ |= ImGuiNodesConnectorStateFlag_Hovered;
                        continue;
                    }

                    if (input.state_ & ImGuiNodesConnectorStateFlag_Consider)
                        input.state_ |= ImGuiNodesConnectorStateFlag_Hovered;
                }
            }

            ////////////////////////////////////////////////////////////////////////////////

            for (int output_idx = 0; output_idx < node->outputs_.size(); ++output_idx)
            {
                ImGuiNodesOutput &output = node->outputs_[output_idx];

                if (output.type_ == ImGuiNodesConnectorType_None)
                    continue;

                output.state_ &= ~(ImGuiNodesConnectorStateFlag_Hovered | ImGuiNodesConnectorStateFlag_Consider | ImGuiNodesConnectorStateFlag_Draging);

                if (state_ == ImGuiNodesState_DragingOutput)
                {
                    if (&output == element_output_)
                        output.state_ |= ImGuiNodesConnectorStateFlag_Draging;

                    continue;
                }

                if (state_ == ImGuiNodesState_DragingInput)
                {
                    if (element_node_ == node)
                        continue;

                    if (ConnectionMatrix(element_node_, node, element_input_, &output))
                        output.state_ |= ImGuiNodesConnectorStateFlag_Consider;
                }

                if (!hovered_node || hovered_node != node)
                    continue;

                if (state_ == ImGuiNodesState_Selecting)
                    continue;

                if (state_ != ImGuiNodesState_DragingInput && node->state_ & ImGuiNodesNodeStateFlag_Selected)
                    continue;

                ImRect output_rect = output.area_output_;
                output_rect.Min *= scale_;
                output_rect.Max *= scale_;
                output_rect.Translate(offset);

                if (output_rect.Contains(mouse_))
                {
                    if (state_ != ImGuiNodesState_DragingInput)
                    {
                        output.state_ |= ImGuiNodesConnectorStateFlag_Hovered;
                        continue;
                    }

                    if (output.state_ & ImGuiNodesConnectorStateFlag_Consider)
                        output.state_ |= ImGuiNodesConnectorStateFlag_Hovered;
                }
            }
        }

        if (hovered_node)
            hovered_node->state_ |= ImGuiNodesNodeStateFlag_Hovered;

        return hovered_node;
    }

    ImGuiNodesNode *ImGuiNodes::CreateNodeFromDesc(ImGuiNodesNodeDesc *desc, ImVec2 pos)
    {
        IM_ASSERT(desc);
        ImGuiNodesNode *node = new ImGuiNodesNode(desc->name_, desc->type_, desc->color_);

        ImVec2 inputs;
        ImVec2 outputs;

        ////////////////////////////////////////////////////////////////////////////////

        for (int input_idx = 0; input_idx < desc->inputs_.size(); ++input_idx)
        {
            ImGuiNodesInput input(desc->inputs_[input_idx].name_, desc->inputs_[input_idx].type_);

            inputs.x = ImMax(inputs.x, input.area_input_.GetWidth());
            inputs.y += input.area_input_.GetHeight();
            node->inputs_.push_back(input);
        }

        for (int output_idx = 0; output_idx < desc->outputs_.size(); ++output_idx)
        {
            ImGuiNodesOutput output(desc->outputs_[output_idx].name_, desc->outputs_[output_idx].type_);

            outputs.x = ImMax(outputs.x, output.area_output_.GetWidth());
            outputs.y += output.area_output_.GetHeight();
            node->outputs_.push_back(output);
        }

        ////////////////////////////////////////////////////////////////////////////////

        node->BuildNodeGeometry(inputs, outputs);
        node->TranslateNode(pos - node->area_node_.GetCenter());
        node->state_ |= ImGuiNodesNodeStateFlag_Visible | ImGuiNodesNodeStateFlag_Hovered | ImGuiNodesNodeStateFlag_Processing;
        node->desc_ = desc;

        ////////////////////////////////////////////////////////////////////////////////

        if (processing_node_)
            processing_node_->state_ &= ~(ImGuiNodesNodeStateFlag_Processing);

        return processing_node_ = node;
    }

    bool ImGuiNodes::SortSelectedNodesOrder()
    {
        bool selected = false;

        std::vector<ImGuiNodesNode *> nodes_unselected;
        nodes_unselected.reserve(nodes_.size());

        std::vector<ImGuiNodesNode *> nodes_selected;
        nodes_selected.reserve(nodes_.size());

        for (ImGuiNodesNode *node : nodes_)
        {
            if (node->state_ & ImGuiNodesNodeStateFlag_Marked || node->state_ & ImGuiNodesNodeStateFlag_Selected)
            {
                selected = true;
                node->state_ &= ~ImGuiNodesNodeStateFlag_Marked;
                node->state_ |= ImGuiNodesNodeStateFlag_Selected;
                nodes_selected.push_back(node);
            }
            else
                nodes_unselected.push_back(node);
        }

        int node_idx = 0;

        for (int unselected_idx = 0; unselected_idx < nodes_unselected.size(); ++unselected_idx)
            nodes_[node_idx++] = nodes_unselected[unselected_idx];

        for (int selected_idx = 0; selected_idx < nodes_selected.size(); ++selected_idx)
            nodes_[node_idx++] = nodes_selected[selected_idx];

        return selected;
    }

    void ImGuiNodes::Update()
    {
        const ImGuiIO &io = ImGui::GetIO();

        UpdateCanvasGeometry(ImGui::GetWindowDrawList());

        ////////////////////////////////////////////////////////////////////////////////

        ImGuiNodesNode *hovered_node = UpdateNodesFromCanvas();

        bool consider_hover = state_ == ImGuiNodesState_Default;
        consider_hover |= state_ == ImGuiNodesState_HoveringNode;
        consider_hover |= state_ == ImGuiNodesState_HoveringInput;
        consider_hover |= state_ == ImGuiNodesState_HoveringOutput;

        ////////////////////////////////////////////////////////////////////////////////

        if (hovered_node && consider_hover)
        {
            element_input_ = NULL;
            element_output_ = NULL;

            for (int input_idx = 0; input_idx < hovered_node->inputs_.size(); ++input_idx)
            {
                if (hovered_node->inputs_[input_idx].state_ & ImGuiNodesConnectorStateFlag_Hovered)
                {
                    element_input_ = &hovered_node->inputs_[input_idx];
                    state_ = ImGuiNodesState_HoveringInput;
                    break;
                }
            }

            for (int output_idx = 0; output_idx < hovered_node->outputs_.size(); ++output_idx)
            {
                if (hovered_node->outputs_[output_idx].state_ & ImGuiNodesConnectorStateFlag_Hovered)
                {
                    element_output_ = &hovered_node->outputs_[output_idx];
                    state_ = ImGuiNodesState_HoveringOutput;
                    break;
                }
            }

            if (!element_input_ && !element_output_)
                state_ = ImGuiNodesState_HoveringNode;
        }

        ////////////////////////////////////////////////////////////////////////////////

        if (state_ == ImGuiNodesState_DragingInput)
        {
            element_output_ = NULL;

            if (hovered_node)
                for (int output_idx = 0; output_idx < hovered_node->outputs_.size(); ++output_idx)
                {
                    ImGuiNodesConnectorState state = hovered_node->outputs_[output_idx].state_;

                    if (state & ImGuiNodesConnectorStateFlag_Hovered && state & ImGuiNodesConnectorStateFlag_Consider)
                        element_output_ = &hovered_node->outputs_[output_idx];
                }
        }

        if (state_ == ImGuiNodesState_DragingOutput)
        {
            element_input_ = NULL;

            if (hovered_node)
                for (int input_idx = 0; input_idx < hovered_node->inputs_.size(); ++input_idx)
                {
                    ImGuiNodesConnectorState state = hovered_node->inputs_[input_idx].state_;

                    if (state & ImGuiNodesConnectorStateFlag_Hovered && state & ImGuiNodesConnectorStateFlag_Consider)
                        element_input_ = &hovered_node->inputs_[input_idx];
                }
        }

        ////////////////////////////////////////////////////////////////////////////////

        if (consider_hover)
        {
            element_node_ = hovered_node;

            if (!hovered_node)
                state_ = ImGuiNodesState_Default;
        }

        ////////////////////////////////////////////////////////////////////////////////

        if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
        {
            switch (state_)
            {
            case ImGuiNodesState_Default:
            {
                bool selected = false;

                for (int node_idx = 0; node_idx < nodes_.size(); ++node_idx)
                {
                    ImGuiNodesState &state = nodes_[node_idx]->state_;

                    if (state & ImGuiNodesNodeStateFlag_Selected)
                        selected = true;

                    state &= ~(ImGuiNodesNodeStateFlag_Selected | ImGuiNodesNodeStateFlag_Marked | ImGuiNodesNodeStateFlag_Hovered);
                }

                if (processing_node_ && false == selected)
                {
                    processing_node_->state_ &= ~(ImGuiNodesNodeStateFlag_Processing);
                    processing_node_ = NULL;
                }

                return;
            };

            case ImGuiNodesState_HoveringInput:
            {
                if (element_input_->target_)
                {
                    element_input_->output_->connections_--;
                    element_input_->output_ = NULL;
                    element_input_->target_ = NULL;

                    state_ = ImGuiNodesState_DragingInput;
                }

                return;
            }

            case ImGuiNodesState_HoveringNode:
            {
                IM_ASSERT(element_node_);

                if (element_node_->state_ & ImGuiNodesNodeStateFlag_Collapsed)
                {
                    element_node_->state_ &= ~ImGuiNodesNodeStateFlag_Collapsed;
                    element_node_->area_node_.Max.y += element_node_->body_height_;
                    element_node_->TranslateNode(ImVec2(0.0f, element_node_->body_height_ * -0.5f));
                }
                else
                {
                    element_node_->state_ |= ImGuiNodesNodeStateFlag_Collapsed;
                    element_node_->area_node_.Max.y -= element_node_->body_height_;

                    // const ImVec2 click = (mouse_ - scroll_ - pos_) / scale_;
                    // const ImVec2 position = click - element_node_->area_node_.GetCenter();

                    element_node_->TranslateNode(ImVec2(0.0f, element_node_->body_height_ * 0.5f));
                }

                state_ = ImGuiNodesState_Draging;
                return;
            }
            }
        }

        ////////////////////////////////////////////////////////////////////////////////

        if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Right))
        {
            switch (state_)
            {
            case ImGuiNodesState_HoveringNode:
            {
                IM_ASSERT(hovered_node);

                if (hovered_node->state_ & ImGuiNodesNodeStateFlag_Disabled)
                    hovered_node->state_ &= ~(ImGuiNodesNodeStateFlag_Disabled);
                else
                    hovered_node->state_ |= (ImGuiNodesNodeStateFlag_Disabled);

                return;
            }
            }
        }

        ////////////////////////////////////////////////////////////////////////////////

        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        {
            switch (state_)
            {
            case ImGuiNodesState_HoveringNode:
            {
                if (io.KeyCtrl)
                    element_node_->state_ ^= ImGuiNodesNodeStateFlag_Selected;

                if (io.KeyShift)
                    element_node_->state_ |= ImGuiNodesNodeStateFlag_Selected;

                bool selected = element_node_->state_ & ImGuiNodesNodeStateFlag_Selected;

                if (false == selected)
                {
                    if (processing_node_)
                        processing_node_->state_ &= ~(ImGuiNodesNodeStateFlag_Processing);

                    element_node_->state_ |= ImGuiNodesNodeStateFlag_Processing;
                    processing_node_ = element_node_;

                    IM_ASSERT(false == nodes_.empty());

                    if (nodes_.back() != element_node_)
                    {
                        auto iterator = std::find(nodes_.begin(), nodes_.end(), element_node_);
                        nodes_.erase(iterator);
                        nodes_.push_back(element_node_);
                    }
                }
                else
                    SortSelectedNodesOrder();

                state_ = ImGuiNodesState_Draging;
                return;
            }

            case ImGuiNodesState_HoveringInput:
            {
                if (!element_input_->target_)
                    state_ = ImGuiNodesState_DragingInput;
                else
                    state_ = ImGuiNodesState_Draging;

                return;
            }

            case ImGuiNodesState_HoveringOutput:
            {
                state_ = ImGuiNodesState_DragingOutput;
                return;
            }
            }

            return;
        }

        ////////////////////////////////////////////////////////////////////////////////

        if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
        {
            switch (state_)
            {
            case ImGuiNodesState_Default:
            {
                ImRect canvas(pos_, pos_ + size_);
                if (false == canvas.Contains(mouse_))
                    return;

                if (false == io.KeyShift)
                    for (int node_idx = 0; node_idx < nodes_.size(); ++node_idx)
                        nodes_[node_idx]->state_ &= ~(ImGuiNodesNodeStateFlag_Selected | ImGuiNodesNodeStateFlag_Marked);

                state_ = ImGuiNodesState_Selecting;
                return;
            }

            case ImGuiNodesState_Selecting:
            {
                const ImVec2 pos = mouse_ - ImGui::GetMouseDragDelta(0);

                area_.Min = ImMin(pos, mouse_);
                area_.Max = ImMax(pos, mouse_);

                return;
            }

            case ImGuiNodesState_Draging:
            {
                if (element_input_ && element_input_->output_ && element_input_->output_->connections_ > 0)
                    return;

                if (false == (element_node_->state_ & ImGuiNodesNodeStateFlag_Selected))
                    element_node_->TranslateNode(io.MouseDelta / scale_, false);
                else
                    for (int node_idx = 0; node_idx < nodes_.size(); ++node_idx)
                        nodes_[node_idx]->TranslateNode(io.MouseDelta / scale_, true);

                return;
            }

            case ImGuiNodesState_DragingInput:
            {
                ImVec2 offset = pos_ + scroll_;
                ImVec2 p1 = offset + (element_input_->pos_ * scale_);
                ImVec2 p4 = element_output_ ? (offset + (element_output_->pos_ * scale_)) : mouse_;

                connection_ = ImVec4(p1.x, p1.y, p4.x, p4.y);
                return;
            }

            case ImGuiNodesState_DragingOutput:
            {
                ImVec2 offset = pos_ + scroll_;
                ImVec2 p1 = offset + (element_output_->pos_ * scale_);
                ImVec2 p4 = element_input_ ? (offset + (element_input_->pos_ * scale_)) : mouse_;

                connection_ = ImVec4(p4.x, p4.y, p1.x, p1.y);
                return;
            }
            }

            return;
        }

        ////////////////////////////////////////////////////////////////////////////////

        if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
        {
            switch (state_)
            {
            case ImGuiNodesState_Selecting:
            {
                element_node_ = NULL;
                element_input_ = NULL;
                element_output_ = NULL;

                area_ = {};

                ////////////////////////////////////////////////////////////////////////////////

                SortSelectedNodesOrder();
                state_ = ImGuiNodesState_Default;
                return;
            }

            case ImGuiNodesState_Draging:
            {
                state_ = ImGuiNodesState_HoveringNode;
                return;
            }

            case ImGuiNodesState_DragingInput:
            case ImGuiNodesState_DragingOutput:
            {
                if (element_input_ && element_output_)
                {
                    IM_ASSERT(hovered_node);
                    IM_ASSERT(element_node_);
                    element_input_->target_ = state_ == ImGuiNodesState_DragingInput ? hovered_node : element_node_;

                    if (element_input_->output_)
                        element_input_->output_->connections_--;

                    element_input_->output_ = element_output_;
                    element_output_->connections_++;
                }

                connection_ = ImVec4();
                state_ = ImGuiNodesState_Default;
                return;
            }
            }

            return;
        }

        ////////////////////////////////////////////////////////////////////////////////

        if (ImGui::IsKeyPressed(ImGuiKey_Delete))
        {
            std::vector<ImGuiNodesNode *> nodes;
            nodes.reserve(nodes_.size());

            for (int node_idx = 0; node_idx < nodes_.size(); ++node_idx)
            {
                ImGuiNodesNode *node = nodes_[node_idx];
                IM_ASSERT(node);

                if (node->state_ & ImGuiNodesNodeStateFlag_Selected)
                {
                    element_node_ = NULL;
                    element_input_ = NULL;
                    element_output_ = NULL;

                    state_ = ImGuiNodesState_Default;

                    for (int sweep_idx = 0; sweep_idx < nodes_.size(); ++sweep_idx)
                    {
                        ImGuiNodesNode *sweep = nodes_[sweep_idx];
                        IM_ASSERT(sweep);

                        for (int input_idx = 0; input_idx < sweep->inputs_.size(); ++input_idx)
                        {
                            ImGuiNodesInput &input = sweep->inputs_[input_idx];

                            if (node == input.target_)
                            {
                                if (input.output_)
                                    input.output_->connections_--;

                                input.target_ = NULL;
                                input.output_ = NULL;
                            }
                        }
                    }

                    for (int input_idx = 0; input_idx < node->inputs_.size(); ++input_idx)
                    {
                        ImGuiNodesInput &input = node->inputs_[input_idx];

                        if (input.output_)
                            input.output_->connections_--;

                        input.type_ = ImGuiNodesNodeType_None;
                        input.name_ = NULL;
                        input.target_ = NULL;
                        input.output_ = NULL;
                    }

                    for (int output_idx = 0; output_idx < node->outputs_.size(); ++output_idx)
                    {
                        ImGuiNodesOutput &output = node->outputs_[output_idx];
                        IM_ASSERT(output.connections_ == 0);
                    }

                    if (node == processing_node_)
                        processing_node_ = NULL;

                    delete node;
                }
                else
                {
                    nodes.push_back(node);
                }
            }

            nodes_ = nodes;

            return;
        }
    }

    void ImGuiNodes::ProcessNodes()
    {
        ImDrawList *draw_list = ImGui::GetWindowDrawList();

        const ImVec2 offset = pos_ + scroll_;

        ////////////////////////////////////////////////////////////////////////////////

        ImGui::SetWindowFontScale(scale_);

        ImVec2 canvasMin = ImGui::GetWindowContentRegionMin() + ImGui::GetWindowPos() + ImVec2{1.f, 1.f};
        ImVec2 canvasMax = ImGui::GetWindowContentRegionMax() + ImGui::GetWindowPos() - ImVec2{1.f, 1.f};
        ImGui::PushClipRect(canvasMin, canvasMax, false);

        for (int node_idx = 0; node_idx < nodes_.size(); ++node_idx)
        {
            const ImGuiNodesNode *node = nodes_[node_idx];
            IM_ASSERT(node);

            for (int input_idx = 0; input_idx < node->inputs_.size(); ++input_idx)
            {
                const ImGuiNodesInput &input = node->inputs_[input_idx];

                if (const ImGuiNodesNode *target = input.target_)
                {
                    IM_ASSERT(target);

                    ImVec2 p1 = offset;
                    ImVec2 p4 = offset;

                    if (node->state_ & ImGuiNodesNodeStateFlag_Collapsed)
                    {
                        ImVec2 collapsed_input = {0, (node->area_node_.Max.y - node->area_node_.Min.y) * 0.5f};

                        p1 += ((node->area_node_.Min + collapsed_input) * scale_);
                    }
                    else
                    {
                        p1 += (input.pos_ * scale_);
                    }

                    if (target->state_ & ImGuiNodesNodeStateFlag_Collapsed)
                    {
                        ImVec2 collapsed_output = {0, (target->area_node_.Max.y - target->area_node_.Min.y) * 0.5f};

                        p4 += ((target->area_node_.Max - collapsed_output) * scale_);
                    }
                    else
                    {
                        p4 += (input.output_->pos_ * scale_);
                    }

                    DrawConnection(p1, p4, ImColor(1.0f, 1.0f, 1.0f, 1.0f));
                }
            }
        }

        for (int node_idx = 0; node_idx < nodes_.size(); ++node_idx)
        {
            const ImGuiNodesNode *node = nodes_[node_idx];
            IM_ASSERT(node);
            node->DrawNode(draw_list, offset, scale_, state_);
        }

        if (connection_.x != connection_.z && connection_.y != connection_.w)
            DrawConnection(ImVec2(connection_.x, connection_.y), ImVec2(connection_.z, connection_.w), ImColor(0.0f, 1.0f, 0.0f, 1.0f));

        ImGui::PopClipRect();

        ImGui::SetWindowFontScale(1.0f);

        ////////////////////////////////////////////////////////////////////////////////

        if (state_ == ImGuiNodesState_Selecting)
        {
            draw_list->AddRectFilled(area_.Min, area_.Max, ImColor(1.0f, 1.0f, 0.0f, 0.1f));
            draw_list->AddRect(area_.Min, area_.Max, ImColor(1.0f, 1.0f, 0.0f, 0.5f));
        }

        ////////////////////////////////////////////////////////////////////////////////

        const char *text, *text_end;

        canvasMin += ImVec2{20.f, 16.f};
        ImFormatStringToTempBuffer(&text, &text_end, "Mouse: %.2f, %.2f", mouse_.x, mouse_.y);
        draw_list->AddText(canvasMin, ImColor(1.0f, 1.0f, 1.0f, 1.0f), text, text_end);

        canvasMin.y += ImGui::GetFontSize() + 8.f;
        ImFormatStringToTempBuffer(&text, &text_end, "Scroll: %.2f, %.2f", scroll_.x, scroll_.y);
        draw_list->AddText(canvasMin, ImColor(1.0f, 1.0f, 1.0f, 1.0f), text, text_end);

        canvasMin.y += ImGui::GetFontSize() + 8.f;
        ImFormatStringToTempBuffer(&text, &text_end, "Scale: %.2f", scale_);
        draw_list->AddText(canvasMin, ImColor(1.0f, 1.0f, 1.0f, 1.0f), text, text_end);

        ////////////////////////////////////////////////////////////////////////////////

        if (nullptr != processing_node_)
        {
            canvasMin.y += ImGui::GetFontSize();

            canvasMin.y += ImGui::GetFontSize() + 8.f;
            if (ImGuiNodesNamesMaxLen > strlen(processing_node_->name_))
                ImFormatStringToTempBuffer(&text, &text_end, "Node: %s", processing_node_->name_);
            else
                ImFormatStringToTempBuffer(&text, &text_end, "Node: %.27s...", processing_node_->name_);
            draw_list->AddText(canvasMin, ImColor(1.0f, 1.0f, 1.0f, 1.0f), text, text_end);

            canvasMin.y += ImGui::GetFontSize() + 8.f;
            ImFormatStringToTempBuffer(&text, &text_end, "Position: %.2f %.2f", processing_node_->area_node_.Min.x, processing_node_->area_node_.Min.y);
            draw_list->AddText(canvasMin, ImColor(1.0f, 1.0f, 1.0f, 1.0f), text, text_end);
        }

        ////////////////////////////////////////////////////////////////////////////////
    }

    void ImGuiNodes::ProcessContextMenu()
    {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));

        if (ImGui::BeginPopup("NodesContextMenu"))
        {
            for (const auto &node_desc : nodes_desc_)
            {
                if (ImGui::MenuItem(node_desc.name_))
                {
                    ImVec2 position = (mouse_ - scroll_ - pos_) / scale_;
                    ImGuiNodesNode *node = CreateNodeFromDesc(const_cast<ImGuiNodesNodeDesc *>(&node_desc), position);
                    nodes_.push_back(node);
                }
            }

            ImGui::EndPopup();
        }

        ImGui::PopStyleVar();
    }

    void ImGuiNodes::AddNodeDesc(ImGuiNodesNodeDesc &&desc)
    {
        nodes_desc_.insert(desc);
    }

    ImGuiNodesNode *ImGuiNodes::AddNode(const std::string_view &desc_name, ImVec2 pos)
    {
        if (desc_name.length() >= ImGuiNodesNamesMaxLen)
            return nullptr;

        ImGui::ImGuiNodesNodeDesc search_desc;
        strcpy_s(search_desc.name_, desc_name.data());

        auto it = nodes_desc_.find(search_desc);
        if (it == nodes_desc_.end())
            return nullptr;

        ImGuiNodesNode *node = CreateNodeFromDesc(const_cast<ImGuiNodesNodeDesc *>(&*it), pos);
        nodes_.push_back(node);
        return node;
    }

    void ImGuiNodes::RemoveNode(ImGuiNodesNode *node)
    {
        element_node_ = nullptr;
        element_input_ = nullptr;
        element_output_ = nullptr;

        if (processing_node_ == node)
            processing_node_ = nullptr;

        nodes_.erase(std::remove(nodes_.begin(), nodes_.end(), node), nodes_.end());
    }

    void ImGuiNodes::Clear()
    {
        nodes_.clear();
    }
}

namespace ImGui
{
    void ImGuiNodesNode::SetName(const char *name)
    {
        name_ = name;

        area_name_.Min = ImVec2(0.0f, 0.0f);
        area_name_.Max = ImGui::CalcTextSize(name);
        title_height_ = ImGuiNodesTitleHight * area_name_.GetHeight();

        ////////////////////////////////////////////////////////////////////////////////

        ImVec2 inputs;
        ImVec2 outputs;

        for (int input_idx = 0; input_idx < desc_->inputs_.size(); ++input_idx)
        {
            ImGuiNodesInput input(desc_->inputs_[input_idx].name_, desc_->inputs_[input_idx].type_);

            inputs.x = ImMax(inputs.x, input.area_input_.GetWidth());
            inputs.y += input.area_input_.GetHeight();
        }

        for (int output_idx = 0; output_idx < desc_->outputs_.size(); ++output_idx)
        {
            ImGuiNodesOutput output(desc_->outputs_[output_idx].name_, desc_->outputs_[output_idx].type_);

            outputs.x = ImMax(outputs.x, output.area_output_.GetWidth());
            outputs.y += output.area_output_.GetHeight();
        }

        ////////////////////////////////////////////////////////////////////////////////

        BuildNodeGeometry(inputs, outputs);
    }

    void ImGuiNodesNode::ToggleCollapse()
    {
        if (state_ & ImGuiNodesNodeStateFlag_Collapsed)
        {
            state_ &= ~ImGuiNodesNodeStateFlag_Collapsed;
            area_node_.Max.y += body_height_;
            TranslateNode(ImVec2(0.0f, body_height_ * -0.5f));
        }
        else
        {
            state_ |= ImGuiNodesNodeStateFlag_Collapsed;
            area_node_.Max.y -= body_height_;

            TranslateNode(ImVec2(0.0f, body_height_ * 0.5f));
        }
    }
}