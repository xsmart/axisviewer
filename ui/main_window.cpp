#include "main_window.h"
#include "../camera_viewer.h"
#include "../image_processing.h"

#include <thread>
#include <stdexcept>

options_widget::options_widget() {
    spinbuttons = new Gtk::SpinButton[6];
    color_mode.append("HSL");
    color_mode.append("HSI");
    color_mode.append("HSV");
    color_mode.append("RGB");
    color_mode.set_active_text("HSL");
    grid.attach(color_mode, 0, 0, 2, 1);
    for (int i = 0; i < 6; i++) {
        spinbuttons[i].set_range(0, 255);
        spinbuttons[i].set_increments(1, 16);
        grid.attach(spinbuttons[i], i % 2, (i/2)+1, 1, 1);
    }
    
}

options_widget::~options_widget() {
    delete[] spinbuttons;
}

COLOR_SPACE options_widget::color_space() {
    Glib::ustring s = color_mode.get_active_text();
    COLOR_SPACE ret;
    if (s == "HSL") {
        ret = COLOR_SPACE::HSL;
    }
    else if (s == "HSI") {
        ret = COLOR_SPACE::HSI;
    }
    else if (s == "HSV") {
        ret = COLOR_SPACE::HSV;
    }
    else if (s == "RGB") {
        ret = COLOR_SPACE::RGB;
    }
    else {
        throw std::runtime_error("Invalid value from ComboBoxText");
    }
    return ret;
}

guint8 options_widget::ch1_low() {
    return (guint8)spinbuttons[0].get_value();
}

guint8 options_widget::ch1_high() {
    return (guint8)spinbuttons[1].get_value();
}

guint8 options_widget::ch2_low() {
    return (guint8)spinbuttons[2].get_value();
}

guint8 options_widget::ch2_high() {
    return (guint8)spinbuttons[3].get_value();
}

guint8 options_widget::ch3_low() {
    return (guint8)spinbuttons[4].get_value();
}

guint8 options_widget::ch3_high() {
    return (guint8)spinbuttons[5].get_value();
}

Gtk::Widget& options_widget::widget() {
    return grid;
}
    
bool main_window::idle_func() {
    if (trans.new_image()) {
        picture = trans.get_image(); //smart pointer
        //now we have shared ownership - we will have sole ownership when
        //trans gets a new image
        processed_picture = picture->copy();
        color replace_false = make_color(0, 0, 0);
        color replace_true = make_color(255, 0, 0);
        color low = make_color(opts.ch1_low(), opts.ch2_low(), opts.ch3_low());
        color high = make_color(opts.ch1_high(), opts.ch2_high(), opts.ch3_high());
        threshold_image(processed_picture, low, high, replace_false, replace_true, opts.color_space());
        image.set(picture);
        processed_image.set(processed_picture);
    }
    return true;
}

bool main_window::have_comm() {
    return trans.has_comm();
}

main_window::main_window() :
    transport_thread(camera_thread, std::ref(trans)),
    img_frame("Images"),
    opt_frame("Processing Options")
{
    //set border width
    set_border_width(10);
    //setup images
    img_grid.set_orientation(Gtk::ORIENTATION_HORIZONTAL);
    img_grid.add(image);
    img_grid.add(processed_image);
    img_frame.add(img_grid);
    //setup options
    opt_frame.add(opts.widget());
    //setup main grid
    main_window_grid.set_orientation(Gtk::ORIENTATION_HORIZONTAL);
    main_window_grid.add(img_frame);
    main_window_grid.add(opt_frame);
    add(main_window_grid);
    show_all();
    Glib::signal_idle().connect(sigc::mem_fun(*this, &main_window::idle_func));
}

main_window::~main_window() {
    //
}