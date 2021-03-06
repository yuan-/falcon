/**
 *  \file gtk_VButtonBox.cpp
 */

#include "gtk_VButtonBox.hpp"

#include <gtk/gtk.h>

/*#
   @beginmodule gtk
*/

namespace Falcon {
namespace Gtk {

/**
 *  \brief module init
 */
void VButtonBox::modInit( Falcon::Module* mod )
{
    Falcon::Symbol* c_VButtonBox = mod->addClass( "GtkVButtonBox", &VButtonBox::init );

    Falcon::InheritDef* in = new Falcon::InheritDef( mod->findGlobalSymbol( "GtkButtonBox" ) );
    c_VButtonBox->getClassDef()->addInheritance( in );

    c_VButtonBox->getClassDef()->factory( &VButtonBox::factory );

#if 0
    Gtk::MethodTab methods[] =
    {
    { "get_spacing_default",    &VButtonBox::get_spacing_default },
    { "get_layout_default",     &VButtonBox::get_layout_default },
    { "set_spacing_default",    &VButtonBox::set_spacing_default },
    { "set_layout_default",     &VButtonBox::set_layout_default },
    { NULL, NULL }
    };

    for ( Gtk::MethodTab* meth = methods; meth->name; ++meth )
        mod->addClassMethod( c_VButtonBox, meth->name, meth->cb );
#endif
}


VButtonBox::VButtonBox( const Falcon::CoreClass* gen, const GtkVButtonBox* box )
    :
    Gtk::CoreGObject( gen, (GObject*) box )
{}


Falcon::CoreObject* VButtonBox::factory( const Falcon::CoreClass* gen, void* box, bool )
{
    return new VButtonBox( gen, (GtkVButtonBox*) box );
}


/*#
    @class GtkVButtonBox
    @brief Vertical button box container.

    A button box should be used to provide a consistent layout of buttons throughout
    your application. The layout/spacing can be altered by the programmer,
    or if desired, by the user to alter the 'feel' of a program to a small degree.
 */
FALCON_FUNC VButtonBox::init( VMARG )
{
#ifndef NO_PARAMETER_CHECK
    if ( vm->paramCount() )
        throw_require_no_args();
#endif
    GtkWidget* wdt = gtk_vbutton_box_new();
    MYSELF;
    self->setObject( (GObject*) wdt );
}


//FALCON FUNC get_spacing_default( VMARG );

//FALCON_FUNC get_layout_default( VMARG );

//FALCON_FUNC set_spacing_default( VMARG );

//FALCON_FUNC set_layout_default( VMARG );


} // Gtk
} // Falcon

// vi: set ai et sw=4:
// kate: replace-tabs on; shift-width 4;
