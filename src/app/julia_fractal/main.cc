/*
 * \brief  Displays julia sets and responds to mode resize events
 * \author Daniel Collins
 * \date   2018-06-22
 *
 *  Creates a pretty animation by drawing successive julia set representations,
 *  periodically varying a constant factor.
 *
 *  The program responds to mode changes. Hence, one can resize the Julia window.
 */

/*
 * Copyright (C) 2018 Daniel Collins
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include <base/component.h>
#include <base/log.h>
#include <os/pixel_rgb888.h>
#include <base/attached_dataspace.h>
#include <timer_session/connection.h>
#include <gui_session/connection.h>
#include <libc/component.h>
#include <complex> // STL

struct Painter_T {
  virtual ~Painter_T() = default;
  virtual void paint(Genode::Pixel_rgb888*, unsigned w, unsigned h) = 0;
};

class window {
  using Reattachable_dataspace = Genode::Constructible<Genode::Attached_dataspace>;

  Genode::Env&           _env;
  Gui::Connection        _npconn;
  Gui::Rect              _gui_win;
  Painter_T&             _draw;
  Reattachable_dataspace _ds{};
  Gui::Top_level_view    _view { _npconn };

  void _draw_frame() {
    _draw.paint(_ds->local_addr<Genode::Pixel_rgb888>(), _gui_win.w(), _gui_win.h()); }

  void _refresh() {
    _npconn.framebuffer.refresh({ { 0, 0 }, _gui_win.area }); }

  void _new_mode() {
    _gui_win = _npconn.window().convert<Gui::Rect>(
      [&] (Gui::Rect rect) { return rect; },
      [&] (Gui::Undefined) { return Gui::Rect { { }, { 256, 256 } }; });
    _npconn.buffer({ .area = _gui_win.area, .alpha = false });
    _ds.construct(_env.rm(), _npconn.framebuffer.dataspace());
    _draw_frame();
    _refresh();
    _view.geometry(_gui_win);
  }

  Genode::Signal_handler<window> _on_resize{_env.ep(), *this, &window::_new_mode};

public:
  using Title_String_T = Genode::String<64>;
 ~window() = default;
  window() = delete;
  window(Genode::Env& env, Painter_T& painter,
         Title_String_T title, Gui::Area wsize)
    : _env{env}, _npconn{env},
      _gui_win{ .at = { }, .area = wsize },
      _draw{painter}
  {
    using Gui::Session;

    _new_mode(); /* Get the newest mode+buffer and start drawing */

    _npconn.enqueue<Session::Command::Title>(_view.id(), title);
    _npconn.execute();
    _npconn.info_sigh(_on_resize);
  }
  
  void draw_next_frame() { 
    _draw_frame();
    _refresh();
  }

};

class julia : public Painter_T {
  using flt_t = double;
  using cmplx = std::complex<flt_t>;
public:

  /** Changed using direct assignment **/
  flt_t              C;
  unsigned           N;

  virtual ~julia() = default;
  julia(flt_t c, unsigned n) :  C{c}, N{n} {}

  /**
   * Draw a calculated set in the buffer. 
   */
  void paint(Genode::Pixel_rgb888* buf, unsigned w, unsigned h) override
  {
    --w, --h;
    const auto _w = w;
    const auto _h = h;
    Genode::size_t idx{0};
    do do {
	/// Scale pixels
	cmplx    Z {w*3.5/_w - 1.75, h*2.0/_h - 1};
	unsigned i {0};
	for (; i < N && (Z.real()*Z.real() + Z.imag()*Z.imag()) < 4.0; ++i)
	  Z = Z*Z + C;

	unsigned c = ((i)*255) / N;
	if (i < N) {
	  double quotient = ((double) i) / N;
	  if (quotient > 0.5)
	    buf[idx++].rgba(255, c, c);
	  else 
	    buf[idx++].rgba(c, 0, 0);
	} else buf[idx++].rgba(0, 0, 0);

    } while (w-- > 0); while (w = _w, h-- > 0);
  }
};

/**
 * Calls a functor on an interval
 */
template <typename Fn_T>
class Timer_callback {
  Genode::Env&      _env;
  Timer::Connection _timer{_env};

  Fn_T _callback;
  void _call() { _callback(); }

  Genode::Signal_handler<Timer_callback<Fn_T>> _time_sigh {
    _env.ep(), *this, &Timer_callback::_call };

public:
  Timer_callback(Genode::Env& env, unsigned interval, Fn_T func)
    : _env{env}, _callback{func} {
      _timer.sigh(_time_sigh);
      _timer.trigger_periodic(interval);
    }
};


void Libc::Component::construct(Libc::Env& env) {
  static julia painter{-.75, 20};
  static window win{env, painter, "julia", Gui::Area{256, 256}};

  auto update_win = [&] {
    painter.C -= 0.003;
    win.draw_next_frame();
  };

  /* Update the window's contents on a static interval. */
  static Timer_callback<decltype(update_win)>
    window_update_timer{env, 15000, update_win};
  Genode::log("constructed");
}
