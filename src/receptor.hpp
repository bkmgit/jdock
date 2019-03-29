#pragma once
#ifndef IDOCK_RECEPTOR_HPP
#define IDOCK_RECEPTOR_HPP

#include <filesystem>
#include "scoring_function.hpp"
#include "atom.hpp"
#include "residue.hpp"
using namespace std::filesystem;

//! Represents a receptor.
class receptor
{
public:
	//! Constructs a receptor by parsing a receptor file in pdbqt format.
	explicit receptor(const path& p);

	//! Constructs a receptor by parsing a receptor file in pdbqt format with a grid map for precalculation being created.
	explicit receptor(const path& p, const array<double, 3>& center, const array<double, 3>& size, const double granularity);

	const bool precise_mode; //!< Precise mode in which no grid map is used.
	const array<double, 3> corner0; //!< Box boundary corner with smallest values of all the 3 dimensions.
	const array<double, 3> corner1; //!< Box boundary corner with largest values of all the 3 dimensions.
	const double granularity; //!< 1D size of grids.
	const double granularity_inverse; //!< 1 / granularity.
	const array<size_t, 3> num_probes; //!< Number of probes.
	const size_t num_probes_product; //!< Product of num_probes[0,1,2].
	vector<atom> atoms; //!< Receptor atoms.
	vector<vector<double>> maps; //!< Grid maps.
	vector<vector<uint16_t>> donors; //!< Indicies of atoms of which contribute to a box grid. The number of atoms is typically less than 65536.
	vector<residue> residues; //!< Receptor residues.

	//! Returns free energy for the given atom type and atom coordinate in precise mode.
	double e(const size_t xs, const array<double, 3>& coord, const scoring_function& sf) const;

	//! Returns free energy for the given atom type and atom coordinate using grid maps.
	inline double e(const size_t xs, const array<double, 3>& coord) const
	{
		return e(xs, index(coord));
	}

	//! Returns free energy for the given atom type and atom index using grid maps.
	inline double e(const size_t xs, const array<size_t, 3>& coord) const
	{
		assert(!precise_mode);
		assert(maps[xs].size());
		return maps[xs][index(coord)];
	}

	//! Returns true if a coordinate is within current half-open-half-close box, i.e. [corner0, corner1).
	bool within(const array<double, 3>& coord) const;

	//! Returns the index of the half-open-half-close grid containing the given coordinate.
	array<size_t, 3> index(const array<double, 3>& coord) const;

	//! Reduces a 3D index to 1D with x being the lowest dimension.
	size_t index(const array<size_t, 3>& idx) const;

	//! Converts 1D index back to a 3D index with x being the lowest dimension.
	array<size_t, 3> coord(const size_t index) const;

	//! Returns the coordinate for the given index of the half-open-half-close grid.
	array<double, 3> coord(const array<size_t, 3>& index) const;

	//! Precalculates auxiliary constants to accelerate grid map creation.
	void precalculate(const vector<size_t>& xs);

	//! Populates grid maps for certain atom types along X and Y dimensions for a given Z dimension value.
	void populate(const vector<size_t>& xs, const size_t z, const scoring_function& sf);

private:
	const array<double, 3> center; //!< Box center.
	const array<double, 3> size; //!< 3D sizes of box.
	vector<vector<size_t>> p_offset; //!< Auxiliary precalculated constants to accelerate grid map creation.

	void parse_pdbqt(const path& p);
};

#endif
