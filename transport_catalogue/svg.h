#pragma once

#include <cstdint>
#include <iostream>
#include <vector>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <variant>

namespace svg {

	struct Rgb {
		Rgb() = default;
		Rgb(std::uint8_t r, std::uint8_t g, std::uint8_t b);
		std::uint8_t red = 0;
		std::uint8_t green = 0;
		std::uint8_t blue = 0;
	};

	struct Rgba : public Rgb {
		Rgba() = default;
		Rgba(std::uint8_t r, std::uint8_t g, std::uint8_t b, double o);
		double opacity = 1.0;
	};

	using Color = std::variant<std::monostate, std::string, svg::Rgb, svg::Rgba>;

	inline const Color NoneColor{ "none" };

	struct ColorPrinter {
		std::ostream& out;
		void operator()(std::monostate) const;
		void operator()(const std::string& color) const;
		void operator()(const svg::Rgb& color) const;
		void operator()(const svg::Rgba& color) const;
	};

	std::ostream& operator<<(std::ostream& os, const svg::Color& color);

	enum class StrokeLineCap {
		BUTT,
		ROUND,
		SQUARE
	};

	enum class StrokeLineJoin {
		ARCS,
		BEVEL,
		MITER,
		MITER_CLIP,
		ROUND
	};

	std::ostream& operator<<(std::ostream& os, const StrokeLineCap line_cap);

	std::ostream& operator<<(std::ostream& os, const StrokeLineJoin line_join);

	struct Point {
		Point() = default;
		Point(double x, double y)
			: x(x)
			, y(y) {
		}
		double x = 0;
		double y = 0;
	};

	/*
	 * Вспомогательная структура, хранящая контекст для вывода SVG-документа с отступами.
	 * Хранит ссылку на поток вывода, текущее значение и шаг отступа при выводе элемента
	 */
	struct RenderContext {
		RenderContext(std::ostream& out)
			: out(out) {
		}

		RenderContext(std::ostream& out, int indent_step, int indent = 0)
			: out(out)
			, indent_step(indent_step)
			, indent(indent) {
		}

		RenderContext Indented() const {
			return { out, indent_step, indent + indent_step };
		}

		void RenderIndent() const {
			for (int i = 0; i < indent; ++i) {
				out.put(' ');
			}
		}

		std::ostream& out;
		int indent_step = 0;
		int indent = 0;
	};

	template <typename Owner>
	class PathProps {
	public:
		Owner& SetFillColor(Color color) {
			fill_color_ = color;
			return AsOwner();
		}

		Owner& SetStrokeColor(Color color) {
			stroke_color_ = color;
			return AsOwner();
		}

		Owner& SetStrokeWidth(double width) {
			stroke_width_ = width;
			return AsOwner();
		}

		Owner& SetStrokeLineCap(StrokeLineCap line_cap) {
			stroke_line_cap_ = line_cap;
			return AsOwner();
		}

		Owner& SetStrokeLineJoin(StrokeLineJoin line_join) {
			stroke_line_join_ = line_join;
			return AsOwner();
		}

		virtual ~PathProps() = default;

	protected:
		template <typename AttrType>
		void RenderAttr(std::ostream& out, const std::string_view attr_name, const std::optional<AttrType>& attr) const {
			using namespace std::literals;
			if (attr) {
				out << " "sv << attr_name << "=\""sv << *attr << "\""sv;
			}
		}

		void RenderAttrs(std::ostream& out) const {
			using namespace std::literals;
			RenderAttr(out, "fill"sv, fill_color_);
			RenderAttr(out, "stroke"sv, stroke_color_);
			RenderAttr(out, "stroke-width"sv, stroke_width_);
			RenderAttr(out, "stroke-linecap"sv, stroke_line_cap_);
			RenderAttr(out, "stroke-linejoin"sv, stroke_line_join_);
		}

	private:
		Owner& AsOwner() {
			return static_cast<Owner&>(*this);
		}

		std::optional<Color> fill_color_;
		std::optional<Color> stroke_color_;
		std::optional<double> stroke_width_;
		std::optional<StrokeLineCap> stroke_line_cap_;
		std::optional<StrokeLineJoin> stroke_line_join_;
	};

	/*
	 * Абстрактный базовый класс Object служит для унифицированного хранения
	 * конкретных тегов SVG-документа
	 * Реализует паттерн "Шаблонный метод" для вывода содержимого тега
	 */
	class Object {
	public:
		void Render(const RenderContext& context) const;

		virtual ~Object() = default;

	private:
		virtual void RenderObject(const RenderContext& context) const = 0;
	};

	class ObjectContainer {
	public:
		template <typename Obj>
		void Add(Obj obj) {
			objects_.emplace_back(std::make_unique<Obj>(std::move(obj)));
		}
		ObjectContainer() = default;
		ObjectContainer(ObjectContainer&& other) noexcept;
		virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;
		virtual ~ObjectContainer() = default;
	protected:
		std::vector<std::unique_ptr<Object>> objects_;
	};

	class Drawable {
	public:
		virtual void Draw(ObjectContainer& container) const = 0;
		virtual ~Drawable() = default;
	};

	/*
	 * Класс Circle моделирует элемент <circle> для отображения круга
	 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
	 */
	class Circle final : public Object, public PathProps<Circle> {
	public:
		Circle& SetCenter(Point center);
		Circle& SetRadius(double radius);

	private:
		void RenderObject(const RenderContext& context) const override;

		Point center_;
		double radius_ = 1.0;
	};

	/*
	 * Класс Polyline моделирует элемент <polyline> для отображения ломаных линий
	 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
	 */
	class Polyline final : public Object, public PathProps<Polyline> {
	public:
		// Добавляет очередную вершину к ломаной линии
		Polyline& AddPoint(Point point);

	private:
		void RenderObject(const RenderContext& context) const override;

		std::vector<Point> points_;
	};

	/*
	 * Класс Text моделирует элемент <text> для отображения текста
	 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
	 */
	class Text final : public Object, public PathProps<Text> {
	public:
		// Задаёт координаты опорной точки (атрибуты x и y)
		Text& SetPosition(Point pos);

		// Задаёт смещение относительно опорной точки (атрибуты dx, dy)
		Text& SetOffset(Point offset);

		// Задаёт размеры шрифта (атрибут font-size)
		Text& SetFontSize(uint32_t size);

		// Задаёт название шрифта (атрибут font-family)
		Text& SetFontFamily(std::string font_family);

		// Задаёт толщину шрифта (атрибут font-weight)
		Text& SetFontWeight(std::string font_weight);

		// Задаёт текстовое содержимое объекта (отображается внутри тега text)
		Text& SetData(std::string data);

	private:
		void RenderObject(const RenderContext& context) const override;

		Point pos_, offset_;
		uint32_t size_ = 1;
		std::string font_family_, font_weight_, data_;
	};

	class Document final : public ObjectContainer {
	public:
		// Добавляет в svg-документ объект-наследник svg::Object
		void AddPtr(std::unique_ptr<Object>&& obj) override;

		// Выводит в ostream svg-представление документа
		void Render(std::ostream& out) const;
	};

}  // namespace svg