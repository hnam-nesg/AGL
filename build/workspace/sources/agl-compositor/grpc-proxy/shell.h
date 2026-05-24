/*
 * Copyright © 2022 Collabora, Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#pragma once

#include <memory>

#include "agl-shell-client-protocol.h"

#include "main-grpc.h"

class Shell {
public:
	std::shared_ptr<struct agl_shell> m_shell;
	struct shell_data *m_shell_data;

	Shell(std::shared_ptr<struct agl_shell> shell,
	      struct shell_data *sh_data) :
		m_shell(shell), m_shell_data(sh_data) { }
	void ActivateApp(const std::string &app_id, const std::string &output_name);
	void DeactivateApp(const std::string &app_id);
	void SetAppSplit(const std::string &app_id, uint32_t orientation,
			 uint32_t width, uint32_t sticky, const std::string &output_name);
	void SetAppFloat(const std::string &app_id,
			 int32_t x_pos, int32_t y_pos);
	void SetAppNormal(const std::string &app_id);
	void SetAppFullscreen(const std::string &app_id);
	void SetAppOnOutput(const std::string &app_id, const std::string &output);
	void SetAppPosition(const std::string &app_id, int32_t x, int32_t y);
	void SetAppScale(const std::string &app_id, int32_t width, int32_t height);
};
