#include "domain.h"
#include <deal.II/base/logstream.h>
#include <deal.II/base/utilities.h>
#include <deal.II/grid/grid_generator.h>
#include <deal.II/grid/grid_in.h>
#include <deal.II/grid/tria_iterator.h>
#include <deal.II/grid/tria_accessor.h>
#include <deal.II/grid/grid_tools.h>
#include <deal.II/numerics/vector_tools.h>
#include <deal.II/fe/fe.h>

using namespace std;

template <int dim, int spacedim>
Domain<dim,spacedim>::Domain() :
  search_mesh("MESH", 1)
{
  initialized = false;
}

template <int dim, int spacedim>
Domain<dim,spacedim>::Domain(ParameterHandler &prm) :
  search_mesh("MESH", 1)
{
  reinit(prm);
}

template <int dim, int spacedim>
Domain<dim,spacedim>::~Domain()
{
  //     tria.clear();
}


template <int dim, int spacedim>
void Domain<dim,spacedim>::reinit(ParameterHandler &prm)
{
  deallog.push("DOMAIN");
  parse_parameters(prm);
  deallog << "Generating coarse triangulation." << endl;
  create_mesh();
  output_mesh();
  deallog.pop();
}


template <int dim, int spacedim>
void Domain<dim,spacedim>::declare_parameters(ParameterHandler &prm)
{
  prm.enter_subsection("Domain Parameters");
  prm.declare_entry ("Read domain mesh from file", "false", Patterns::Bool(),
                     "If this is false, then the input mesh file below is ignored and a hyper-cube is created.");
  prm.declare_entry ("Path of domain mesh files", "mesh/", Patterns::Anything());
  prm.declare_entry ("Input mesh file", "square", Patterns::Anything());
  prm.declare_entry ("Input mesh format", "ucd",
                     Patterns::Selection(GridIn<dim>::get_format_names()));
  prm.declare_entry ("Output mesh file", "square_out", Patterns::Anything());

  prm.enter_subsection("Grid Out Parameters");
  GridOut::declare_parameters(prm);
  prm.leave_subsection();

  prm.leave_subsection();

}

template <int dim, int spacedim>
void Domain<dim,spacedim>::parse_parameters(ParameterHandler &prm)
{
  prm.enter_subsection("Domain Parameters");
  read_mesh   = prm.get_bool ("Read domain mesh from file");
  search_mesh.add_path(prm.get ("Path of domain mesh files"));
  input_mesh_file_name = prm.get ("Input mesh file");
  output_mesh_file_name = prm.get ("Output mesh file");
  input_mesh_format = prm.get ("Input mesh format");

  prm.enter_subsection("Grid Out Parameters");
  gridout.parse_parameters(prm);
  prm.leave_subsection();

  prm.leave_subsection();
}

template <int dim, int spacedim>
void Domain<dim,spacedim>::create_mesh()
{
  if (read_mesh)
    {
      GridIn<dim,spacedim> grid_in;
      grid_in.attach_triangulation (tria);
      string mfilen = search_mesh.find
                      (input_mesh_file_name,
                       grid_in.default_suffix(grid_in.parse_format(input_mesh_format)),
                       "r");
      ifstream mfile(mfilen.c_str());
      grid_in.read(mfile, GridIn<dim,spacedim>::parse_format(input_mesh_format));
    }
  else
    {
      Point<spacedim> corner;
      for (unsigned int d=0; d<dim; ++d) corner[d] = 1.;
      GridGenerator::hyper_rectangle (tria, Point<spacedim>(), corner, true);
//        GridTools::partition_triangulation (n_mpi_processes, tria);
    }
  initialized = true;
}


template <int dim, int spacedim>
void Domain<dim,spacedim>::output_mesh(std::ostream &out) const
{
  Assert(initialized, ExcNotInitialized());
  gridout.write (tria, out);
}


template <int dim, int spacedim>
void Domain<dim,spacedim>::output_mesh() const
{
  if (gridout.default_suffix() != "")
    {
      std::ofstream out_file ((output_mesh_file_name + gridout.default_suffix()).c_str());
      output_mesh(out_file);
      out_file.close();
    }
}



template class Domain<1>;
//template class Domain<1,2>;
template class Domain<2>;
//template class Domain<2,3>;
template class Domain<3>;
