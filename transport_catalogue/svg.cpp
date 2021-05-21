#include "svg.h"

using namespace std::literals;

namespace svg {

	Rgb::Rgb(std::uint8_t r, std::uint8_t g, std::uint8_t b)
		: red(r)
		, green(g)
		, blue(b) {
	}

	Rgba::Rgba(std::uint8_t r, std::uint8_t g, std::uint8_t b, double o)
		: Rgb(r, g, b)
		, opacity(o) {
	}

	void ColorPrinter::operator()(std::monostate) const {
		out << std::get<std::string>(svg::NoneColor);
	}

	void ColorPrinter::operator()(const std::string& color) const {
		out << color;
	}

	void ColorPrinter::operator()(const svg::Rgb& color) const {
		using namespace std::literals;
		out << "rgb("sv
			<< static_cast<int>(color.red) << ","sv
			<< static_cast<int>(color.green) << ","sv
			<< static_cast<int>(color.blue)
			<< ")"sv;
	}

	void ColorPrinter::operator()(const svg::Rgba& color) const {
		using namespace std::literals;
		out << "rgba("sv
			<< static_cast<int>(color.red) << ","sv
			<< static_cast<int>(color.green) << ","sv
			<< static_cast<int>(color.blue) << ","sv
			<< color.opacity
			<< ")"sv;
	}

	std::ostream& operator<<(std::ostream& os, const StrokeLineCap line_cap) {
		switch (line_cap) {
		case StrokeLineCap::BUTT:
			os << "butt"sv;
			break;
		case StrokeLineCap::ROUND:
			os << "round"sv;
			break;
		case StrokeLineCap::SQUARE:
			os << "square"sv;
			break;
		}
		return os;
	}

	std::ostream& operator<<(std::ostream& os, const StrokeLineJoin line_join) {
		switch (line_join) {
		case StrokeLineJoin::ARCS:
			os << "arcs"sv;
			break;
		case StrokeLineJoin::BEVEL:
			os << "bevel"sv;
			break;
		case StrokeLineJoin::MITER:
			os << "miter"sv;
			break;
		case StrokeLineJoin::MITER_CLIP:
			os << "miter-clip"sv;
			break;
		case StrokeLineJoin::ROUND:
			os << "round"sv;
			break;
		}
		return os;
	}

	std::ostream& operator<<(std::ostream& os, const svg::Color& color) {
		std::visit(svg::ColorPrinter{ os }, color);
		return os;
	}

	void Object::Render(const RenderContext& context) const {
		context.RenderIndent();

		// Делегируем вывод тега своим подклассам
		RenderObject(context);

		context.out << std::endl;
	}

	ObjectContainer::ObjectContainer(ObjectContainer&& other) noexcept
		: objects_(std::move(other.objects_)) {
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
		out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
		out << "r=\""sv << radius_ << "\""sv;
		RenderAttrs(out);
		out << " />"sv;
	}

	// --------- Polyline -----------------

	Polyline& Polyline::AddPoint(Point point) {
		points_.push_back(point);
		return *this;
	}

	void Polyline::RenderObject(const RenderContext& context) const {
		auto& out = context.out;
		out << "<polyline points=\""sv;
		bool first = true;
		for (const auto& point : points_) {
			if (first) {
				first = false;
			}
			else {
				out << " "sv;
			}
			out << point.x << ","sv << point.y;
		}
		out << "\""sv;
		RenderAttrs(out);
		out << " />"sv;
	}

	// ----------- Text -------------------

	Text& Text::SetPosition(Point pos) {
		pos_ = pos;
		return *this;
	}

	Text& Text::SetOffset(Point offset) {
		offset_ = offset;
		return *this;
	}

	Text& Text::SetFontSize(uint32_t size) {
		size_ = size;
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
		data_ = data;
		return *this;
	}

	void Text::RenderObject(const RenderContext& context) const {
		auto& out = context.out;
		out << "<text "sv;
		out << "x=\""sv << pos_.x << "\" y=\""sv << pos_.y << "\" "sv;
		out << "dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\" "sv;
		out << "font-size=\""sv << size_ << "\""sv;
		if (!font_family_.empty()) {
			out << " font-family=\""sv << font_family_ << "\""sv;
		}
		if (!font_weight_.empty()) {
			out << " font-weight=\""sv << font_weight_ << "\""sv;
		}
		RenderAttrs(out);
		out << ">"sv;
		for (const char c : data_) {
			switch (c) {
			case '\"':
				out << "&quot;"sv;
				break;
			case '\'':
				out << "&apos;"sv;
				break;
			case '<':
				out << "&lt;"sv;
				break;
			case '>':
				out << "&gt;"sv;
				break;
			case '&':
				out << "&amp;"sv;
				break;
			default:
				out << c;
				break;
			}
		}
		out << "</text>"sv;
	}

	// --------- Document -----------------

	void Document::AddPtr(std::unique_ptr<Object>&& obj) {
		objects_.emplace_back(std::move(obj));
	}

	void Document::Render(std::ostream& out) const {
		out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"sv;
		out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n"sv;
		for (const auto& obj : objects_) {
			obj->Render({ out, 1, 2 });
		}
		out << "</svg>"sv;
	}

}  // namespace svg