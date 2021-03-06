#ifndef GTK_CELLRENDERERPIXBUF_HPP
#define GTK_CELLRENDERERPIXBUF_HPP

#include "modgtk.hpp"

#define GET_CELLRENDERERPIXBUF( item ) \
        ((GtkCellRendererPixbuf*)((Gtk::CellRendererPixbuf*) (item).asObjectSafe() )->getObject())


namespace Falcon {
namespace Gtk {

/**
 *  \class Falcon::Gtk::CellRendererPixbuf
 */
class CellRendererPixbuf
    :
    public Gtk::CoreGObject
{
public:

    CellRendererPixbuf( const Falcon::CoreClass*, const GtkCellRendererPixbuf* = 0 );

    static Falcon::CoreObject* factory( const Falcon::CoreClass*, void*, bool );

    static void modInit( Falcon::Module* );

    static FALCON_FUNC init( VMARG );

};


} // Gtk
} // Falcon

#endif // !GTK_CELLRENDERERPIXBUF_HPP

// vi: set ai et sw=4:
// kate: replace-tabs on; shift-width 4;
