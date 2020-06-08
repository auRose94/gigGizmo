#pragma once

#include <html.hpp>
#include <types.hpp>

namespace gg {
	namespace bs {
		using namespace gold;
		using div = HTML::div;
		using option = HTML::option;
		div row(gold::list content);
		div col(gold::list content);
		div formCheck(
			string name, string labelText, object inputAtts = {});
		div formRadio(
			string name, string id, string labelText, string value,
			object inputAtts = {});
		div formInputRow(
			string type, string name, string labelText, bool req,
			string error = "", object inputAtts = {});
		div formInputColumn(
			string type, string name, string labelText, bool req,
			string error = "", object inputAtts = {});
		div formFileInput(
			string id, string labelText, object inputAtts = {});
		div formSelect(
			string name, string labelText, bool req,
			object inputAtts = {}, gold::list content = {});
		option formSelectOption(
			string title, string value, bool selected = false);
	}  // namespace bs
}  // namespace gg