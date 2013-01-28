// generated by Fast Light User Interface Designer (fluid) version 1.0108

#include "gui.h"

void GUI::cb_DBG_i(Fl_Button*, void*) {
  DBG_cb();
}
void GUI::cb_DBG(Fl_Button* o, void* v) {
  ((GUI*)(o->parent()->user_data()))->cb_DBG_i(o,v);
}

void GUI::cb_Save_i(Fl_Button*, void*) {
  SaveView0B_cb();
}
void GUI::cb_Save(Fl_Button* o, void* v) {
  ((GUI*)(o->parent()->user_data()))->cb_Save_i(o,v);
}

void GUI::cb_Load_i(Fl_Button*, void*) {
  LoadView0B_cb();
}
void GUI::cb_Load(Fl_Button* o, void* v) {
  ((GUI*)(o->parent()->user_data()))->cb_Load_i(o,v);
}

void GUI::cb_Go_i(Fl_Button*, void*) {
  GoToView0B_cb();
}
void GUI::cb_Go(Fl_Button* o, void* v) {
  ((GUI*)(o->parent()->user_data()))->cb_Go_i(o,v);
}

void GUI::cb_Save1_i(Fl_Button*, void*) {
  SaveView1B_cb();
}
void GUI::cb_Save1(Fl_Button* o, void* v) {
  ((GUI*)(o->parent()->user_data()))->cb_Save1_i(o,v);
}

void GUI::cb_Load1_i(Fl_Button*, void*) {
  LoadView1B_cb();
}
void GUI::cb_Load1(Fl_Button* o, void* v) {
  ((GUI*)(o->parent()->user_data()))->cb_Load1_i(o,v);
}

void GUI::cb_Go1_i(Fl_Button*, void*) {
  GoToView1B_cb();
}
void GUI::cb_Go1(Fl_Button* o, void* v) {
  ((GUI*)(o->parent()->user_data()))->cb_Go1_i(o,v);
}
#include "scene.h"

GUI::GUI() {
  { uiw = new Fl_Double_Window(265, 290, "GUI");
    uiw->user_data((void*)(this));
    { Fl_Button* o = new Fl_Button(15, 15, 225, 40, "DBG");
      o->selection_color(FL_DARK_RED);
      o->callback((Fl_Callback*)cb_DBG);
    } // Fl_Button* o
    { Fl_Button* o = new Fl_Button(15, 65, 95, 40, "Save View 0");
      o->callback((Fl_Callback*)cb_Save);
    } // Fl_Button* o
    { Fl_Button* o = new Fl_Button(15, 110, 95, 40, "Load View 0");
      o->callback((Fl_Callback*)cb_Load);
    } // Fl_Button* o
    { Fl_Button* o = new Fl_Button(15, 155, 95, 40, "Go To View 0");
      o->callback((Fl_Callback*)cb_Go);
    } // Fl_Button* o
    { Fl_Button* o = new Fl_Button(140, 65, 95, 40, "Save View 1");
      o->callback((Fl_Callback*)cb_Save1);
    } // Fl_Button* o
    { Fl_Button* o = new Fl_Button(140, 110, 95, 40, "Load View 1");
      o->callback((Fl_Callback*)cb_Load1);
    } // Fl_Button* o
    { Fl_Button* o = new Fl_Button(140, 155, 95, 40, "Go To View 1");
      o->callback((Fl_Callback*)cb_Go1);
    } // Fl_Button* o
    uiw->end();
  } // Fl_Double_Window* uiw
}

int main(int argc, char **argv) {
  scene = new Scene;
  return Fl::run();
}

void GUI::show() {
  uiw->show();
}

void GUI::DBG_cb() {
  scene->DBG();
}

void GUI::SaveView0B_cb() {
  scene->SaveView0();
}

void GUI::LoadView0B_cb() {
  scene->LoadView0();
}

void GUI::GoToView0B_cb() {
  scene->GoToView0();
}

void GUI::SaveView1B_cb() {
  scene->SaveView1();
}

void GUI::LoadView1B_cb() {
  scene->LoadView1();
}

void GUI::GoToView1B_cb() {
  scene->GoToView1();
}
