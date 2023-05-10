#include "svg.h"

#include <sstream>

namespace svg {

    using namespace std::literals;

    std::ostream& operator<<(std::ostream& out, const StrokeLineCap& line_cap) {
        switch (line_cap)
        {
        case svg::StrokeLineCap::BUTT:
            out << "butt"sv;
            break;
        case svg::StrokeLineCap::ROUND:
            out << "round"sv;
            break;
        case svg::StrokeLineCap::SQUARE:
            out << "square"sv;
            break;
        default:
            break;
        }
        return out;
    }

    std::ostream& operator<<(std::ostream& out, const StrokeLineJoin& line_join) {
        switch (line_join)
        {
        case svg::StrokeLineJoin::ARCS:
            out << "arcs"sv;
            break;
        case svg::StrokeLineJoin::BEVEL:
            out << "bevel"sv;
            break;
        case svg::StrokeLineJoin::MITER:
            out << "miter"sv;
            break;
        case svg::StrokeLineJoin::MITER_CLIP:
            out << "miter-clip"sv;
            break;
        case svg::StrokeLineJoin::ROUND:
            out << "round"sv;
            break;
        default:
            break;
        }
        return out;
    }

    // ---------- Color ------------------

    std::ostream& operator<<(std::ostream& out, const Color& color) {
        std::ostringstream strm;
        std::visit(ColorPrinter{ strm }, color);
        out << strm.str();
        return out;
    }

    // ---------- Object ------------------


    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();

        // Делегируем вывод тега своим подклассам
        RenderObject(context);

        context.out << std::endl;
    }

    // ---------- Circle ------------------

    Circle& Circle::SetCenter(Point center) {
        center_ = center;
        return *this;
    }

    Circle& Circle::SetRadius(double radius) {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << R"(<circle cx=")" << center_.x << R"(" cy=")" << center_.y << R"(" )";
        out << R"(r=")" << radius_ << R"(" )";
        RenderAttrs(context.out);
        out << "/>"sv;
    }

    // ---------- Polyline ------------------

    Polyline& Polyline::AddPoint(Point point) {
        vertexes_.push_back(point);
        return *this;
    }


    void Polyline::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        //out << "<polyline points=\""sv;
        out << "<polyline points=\""sv;
        bool is_first = true;
        for (auto& vert : vertexes_) {
            if (is_first) {
                out << vert.x << ","sv << vert.y;
                is_first = false;
            }
            else {
                out << " "sv << vert.x << ","sv << vert.y;
            }
        }
        out << "\""sv;
        RenderAttrs(context.out);
        out << "/>"sv;
    }

    // ---------- Text ------------------

    Text& Text::SetPosition(Point pos) {
        pos_ = pos;
        return *this;
    }

    Text& Text::SetOffset(Point offset) {
        offset_ = offset;
        return *this;
    }

    Text& Text::SetFontSize(uint32_t size) {
        font_size_ = size;
        return *this;
    }

    Text& Text::SetFontFamily(std::string font_family) {
        font_family_ = font_family;
        return *this;
    }

    Text& Text::SetFontWeight(std::string font_weight) {
        font_weight_ = font_weight;
        return *this;
    }

    Text& Text::SetData(std::string data) {
        text_data_ = data;
        return *this;
    }

    std::string Text::TransformData() const {
        //text_data_
        std::string transformed_string;
        for (auto c : text_data_) {
            switch (c)
            {
            case '"':
                transformed_string += "&quot;";
                break;
            case '\'':
                transformed_string += "&apos;";
                break;
            case '<':
                transformed_string += "&lt;";
                break;
            case '>':
                transformed_string += "&gt;";
                break;
            case '&':
                transformed_string += "&amp;";
                break;
            default:
                transformed_string += c;
                break;
            }
        }
        return transformed_string;
    }

    void Text::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<text"sv;
        RenderAttrs(context.out);
        out << " x=\""sv << pos_.x << "\" y=\"" << pos_.y << "\" "sv;
        out << "dx=\""sv << offset_.x << "\" dy=\"" << offset_.y << "\" "sv;
        out << "font-size=\""sv << font_size_ << "\""sv;
        if (!font_family_.empty()) {
            out << " font-family=\""sv << font_family_ << "\""sv;
        }
        if (!font_weight_.empty()) {
            out << " font-weight=\""sv << font_weight_ << "\""sv;
        }
        out << ">"sv;
        std::string transformed_data = TransformData();
        out << transformed_data;
        //out << text_data_;
        out << "</text>"sv;
    }

    // ---------- Document ------------------

    void Document::AddPtr(std::unique_ptr<Object>&& obj) {
        //objects_.emplace_back(std::move(obj));
        objects_.push_back(std::move(obj));
    }


    void Document::Render(std::ostream& out) const {
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"sv;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n"sv;
        RenderContext context(out, 2, 2);
        for (const auto& obj : objects_) {
            obj->Render(context);
        }
        //out << "</svg>" << std::endl;
        out << "</svg>";
    }

}  // namespace svg