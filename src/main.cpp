#include <chrono>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <boost/program_options.hpp>
#include "io_service_pool.hpp"
#include "safe_counter.hpp"
#include "random_forest.hpp"
#include "receptor.hpp"
#include "ligand.hpp"

int main(int argc, char* argv[])
{
	path receptor_path, ligand_path, out_path;
	array<double, 3> center, size;
	size_t seed, num_threads, num_trees, num_tasks, max_conformations;
	double granularity;
	bool score_only, both_score_dock, with_rf_score, precise_mode, remove_nonstd;

	// Process program options.
	try
	{
		// Initialize the default values of optional arguments.
		using namespace std::chrono;
		const path default_out_path = ".";
		const size_t default_seed = duration_cast<seconds>(system_clock::now().time_since_epoch()).count();
		const size_t default_num_threads = boost::thread::hardware_concurrency();
		const size_t default_num_trees = 500;
		const size_t default_num_tasks = 64;
		const size_t default_max_conformations = 9;
		const double default_granularity = 0.125;

		// Set up options description.
		using namespace boost::program_options;
		options_description input_options("input (required)");
		input_options.add_options()
			("receptor", value<path>(&receptor_path)->required(), "receptor file in PDBQT format")
			("ligand", value<path>(&ligand_path)->required(), "ligand file or folder of ligands in PDBQT format")
			("center_x", value<double>(&center[0]), "x coordinate of the search space center, not required if both --score_only and --precise_mode are on")
			("center_y", value<double>(&center[1]), "y coordinate of the search space center, not required if both --score_only and --precise_mode are on")
			("center_z", value<double>(&center[2]), "z coordinate of the search space center, not required if both --score_only and --precise_mode are on")
			("size_x", value<double>(&size[0]), "size in the x dimension in Angstrom, not required if both --score_only and --precise_mode are on")
			("size_y", value<double>(&size[1]), "size in the y dimension in Angstrom, not required if both --score_only and --precise_mode are on")
			("size_z", value<double>(&size[2]), "size in the z dimension in Angstrom, not required if both --score_only and --precise_mode are on")
			;
		options_description output_options("output (optional)");
		output_options.add_options()
			("out", value<path>(&out_path)->default_value(default_out_path), "folder of predicted conformations in PDBQT format")
			;
		options_description miscellaneous_options("options (optional)");
		miscellaneous_options.add_options()
			("seed", value<size_t>(&seed)->default_value(default_seed), "explicit non-negative random seed")
			("threads", value<size_t>(&num_threads)->default_value(default_num_threads), "number of worker threads to use")
			("trees", value<size_t>(&num_trees)->default_value(default_num_trees), "number of decision trees in random forest, no effect without --rf_score")
			("tasks", value<size_t>(&num_tasks)->default_value(default_num_tasks), "number of Monte Carlo tasks for global search")
			("conformations", value<size_t>(&max_conformations)->default_value(default_max_conformations), "maximum number of binding conformations to write")
			("granularity", value<double>(&granularity)->default_value(default_granularity), "density of probe atoms of grid maps")
			("score_only", bool_switch(&score_only), "scoring input ligand conformation without docking, this option conflicts with --score_dock")
			("score_dock", bool_switch(&both_score_dock), "scoring input ligand conformation as well as docking, this option conflicts with --score_only")
			("rf_score", bool_switch(&with_rf_score), "compute RF-Score as well")
			("precise_mode", bool_switch(&precise_mode), "precise mode in which no precalculated energy grid map is used, requires --score_only or --score_dock")
			("remove_nonstd", bool_switch(&remove_nonstd), "remove non standard residues from receptor")
			("help", "this help information")
			("version", "version information")
			("config", value<path>(), "configuration file to load options from")
			;
		options_description all_options;
		all_options.add(input_options).add(output_options).add(miscellaneous_options);

		// Parse command line arguments.
		variables_map vm;
		store(parse_command_line(argc, argv, all_options), vm);

		// If no command line argument is supplied or help is requested, print the usage and exit.
		if (argc == 1 || vm.count("help"))
		{
			cout << all_options;
			return 0;
		}

		// If version is requested, print the version and exit.
		if (vm.count("version"))
		{
			cout << "2.2.2b" << endl;
			return 0;
		}

		// If a configuration file is present, parse it.
		if (vm.count("config"))
		{
			ifstream config_file(vm["config"].as<path>());
			store(parse_config_file(config_file, all_options), vm);
		}

		// Notify the user of parsing errors, if any.
		vm.notify();

		// Validate size and center.
		if (!score_only || !precise_mode)
		{
			const string required_options[] = { "center_x", "center_y", "center_z", "size_x", "size_y", "size_z" };
			for (const auto& opt : required_options)
			{
				if (!vm.count(opt))
				{
					cerr << "the option '--" << opt << "' is required but missing" << endl;
					return 1;
				}
			}
		}

		// Validate receptor_path.
		if (!exists(receptor_path))
		{
			cerr << "Option receptor " << receptor_path << " does not exist" << endl;
			return 1;
		}
		if (!is_regular_file(receptor_path))
		{
			cerr << "Option receptor " << receptor_path << " is not a regular file" << endl;
			return 1;
		}

		// Validate ligand_path.
		if (!exists(ligand_path))
		{
			cerr << "Option ligand " << ligand_path << " does not exist" << endl;
			return 1;
		}

		// Validate out_path.
		if (exists(out_path))
		{
			if (!is_directory(out_path))
			{
				cerr << "Option out " << out_path << " is not a directory" << endl;
				return 1;
			}
		}
		else
		{
			if (!create_directories(out_path))
			{
				cerr << "Failed to create output folder " << out_path << endl;
				return 1;
			}
		}

		// Validate miscellaneous options.
		if (!num_threads)
		{
			cerr << "Option threads must be 1 or greater" << endl;
			return 1;
		}
		if (!num_tasks)
		{
			cerr << "Option tasks must be 1 or greater" << endl;
			return 1;
		}
		if (!max_conformations)
		{
			cerr << "Option conformations must be 1 or greater" << endl;
			return 1;
		}
		if (granularity <= 0)
		{
			cerr << "Option granularity must be positive" << endl;
			return 1;
		}
		if (score_only && both_score_dock)
		{
			cerr << "Option --score_only and --score_dock cannot be combined" << endl;
			return 1;
		}
		if (precise_mode && !score_only && !both_score_dock)
		{
			cerr << "Option --precise_mode must be combined with --score_only or --score_dock" << endl;
			return 1;
		}
	}
	catch (const exception& e)
	{
		cerr << e.what() << endl;
		return 1;
	}

	// Parse the receptor.
	cout << "Parsing the receptor " << receptor_path << endl;
	receptor rec = precise_mode && score_only ? receptor(receptor_path, remove_nonstd) : receptor(receptor_path, remove_nonstd, center, size, granularity);
	cout << "Found " << rec.atoms.size() << " atoms in " << rec.residues.size() << " residues in receptor " << receptor_path << endl;

	// Reserve storage for result containers.
	vector<vector<result>> result_containers(num_tasks);
	for (auto& rc : result_containers)
	{
		rc.reserve(20);	// Maximum number of results obtained from a single Monte Carlo task.
	}
	vector<result> results;
	results.reserve(max_conformations);

	// Enumerate and sort input ligands.
	cout << "Enumerating input ligands in " << ligand_path << endl;
	vector<path> input_ligand_paths;
	if (is_regular_file(ligand_path))
	{
		input_ligand_paths.push_back(ligand_path);
	}
	else
	{
		for (directory_iterator dir_iter(ligand_path), end_dir_iter; dir_iter != end_dir_iter; ++dir_iter)
		{
			// Filter files with .pdbqt and .PDBQT extensions.
			const path input_ligand_path = dir_iter->path();
			const auto ext = input_ligand_path.extension();
			if (ext != ".pdbqt" && ext != ".PDBQT") continue;
			input_ligand_paths.push_back(input_ligand_path);
		}
	}
	const size_t num_input_ligands = input_ligand_paths.size();
	cout << "Sorting " << num_input_ligands << " input ligands in alphabetical order" << endl;
	sort(input_ligand_paths.begin(), input_ligand_paths.end());

	// Initialize a Mersenne Twister random number generator.
	cout << "Seeding a random number generator with " << seed << endl;
	mt19937_64 rng(seed);

	// Initialize an io service pool and create worker threads for later use.
	cout << "Creating an io service pool of " << num_threads << " worker threads" << endl;
	io_service_pool io(num_threads);
	safe_counter<size_t> cnt;

	// Precalculate the scoring function in parallel.
	cout << "Calculating a scoring function of " << scoring_function::n << " atom types" << endl;
	scoring_function sf;
	cnt.init((sf.n + 1) * sf.n >> 1);
	for (size_t t1 = 0; t1 < sf.n; ++t1)
		for (size_t t0 = 0; t0 <= t1; ++t0)
		{
			io.post([&, t0, t1]()
			{
				sf.precalculate(t0, t1);
				cnt.increment();
			});
		}
	cnt.wait();
	sf.clear();

	forest f(num_trees, seed);
	if (with_rf_score)
	{
		// Train RF-Score on the fly.
		cout << "Training a random forest of " << num_trees << " trees with " << tree::nv << " variables and " << tree::ns << " samples" << endl;
		cnt.init(num_trees);
		for (size_t i = 0; i < num_trees; ++i)
		{
			io.post([&, i]()
			{
				f[i].train(8, f.u01_s);
				cnt.increment();
			});
		}
		cnt.wait();
		f.clear();
	}

	// Output headers to the standard output and the log file.
	const char separator = '|';
	cout << "Creating grid maps of " << granularity << " A and running " << num_tasks << " Monte Carlo searches per ligand" << endl;
	cout             << setw( 8) << "Index"
		<< separator << setw(16) << "Ligand"
		<< separator << setw( 8) << "Atoms"
		<< separator << setw( 8) << "Torsions"
		<< separator << setw( 6) << "nConfs"
		<< separator << setw(22) << "idock score (kcal/mol)";
	if (with_rf_score)
		cout << separator << setw(14) << "RF-Score (pKd)";
	cout << endl << setprecision(2);
	cout.setf(ios::fixed, ios::floatfield);

	ofstream log(out_path / (receptor_path.stem().string() + ".csv"));
	log.setf(ios::fixed, ios::floatfield);
	log << "Ligand,Atoms,Torsions,nConfs,idock score (kcal/mol)";
	if (with_rf_score)
		log << ",RF-Score (pKd)";
	log << endl << setprecision(2);

	// Start to dock each input ligand.
	size_t index = 0;
	for (const auto& input_ligand_path : input_ligand_paths)
	{
		// Output the ligand file stem.
		const string stem = input_ligand_path.stem().string();
		cout             << setw( 8) << ++index
			<< separator << setw(16) << stem
			<< flush;
		log << stem;

		// Parse the ligand.
		array<double, 3> origin;
		const ligand lig(input_ligand_path, origin);
		cout << separator << setw(8) << lig.num_heavy_atoms
			<< separator << setw(8) << lig.num_active_torsions;
		log << ',' << lig.num_heavy_atoms << ',' << lig.num_active_torsions;

		// Check if the current ligand has already been docked.
		size_t num_confs = 0;
		double id_score = 0;
		double rf_score = 0;
		const path output_ligand_path = out_path / input_ligand_path.filename();
		if (exists(output_ligand_path) && !equivalent(ligand_path, out_path))
		{
			// Extract idock score and RF-Score from output file.
			string line;
			for (ifstream ifs(output_ligand_path); getline(ifs, line);)
			{
				const string record = line.substr(0, 10);
				if (record == "MODEL     ")
				{
					++num_confs;
				}
				else if (num_confs == 1 && record == "REMARK 921")
				{
					id_score = stod(line.substr(55, 8));
				}
				else if (num_confs == 1 && record == "REMARK 927")
				{
					rf_score = stod(line.substr(55, 8));
				}
			}
		}
		else
		{
			// Precise mode uses grid maps if docking is going to perform as well.
			if (rec.use_maps)
			{
				// Find atom types that are present in the current ligand but not present in the grid maps.
				vector<size_t> xs;
				for (size_t t = 0; t < sf.n; ++t)
				{
					if (lig.xs[t] && rec.init_e(t))
					{
						xs.push_back(t);
					}
				}

				// Create grid maps on the fly if necessary.
				if (xs.size())
				{
					// Precalculate p_offset.
					rec.precalculate(xs);

					// Populate the grid map task container.
					cnt.init(rec.num_probes[2]);
					for (size_t z = 0; z < rec.num_probes[2]; ++z)
					{
						io.post([&, z]()
						{
							rec.populate(xs, z, sf);
							cnt.increment();
						});
					}
					cnt.wait();
				}
			}

			vector<bool> mask(rec.residues.size());

			// To dock, search conformations.
			if (!score_only)
			{
				// Run the Monte Carlo tasks.
				cnt.init(num_tasks);
				for (size_t i = 0; i < num_tasks; ++i)
				{
					assert(result_containers[i].empty());
					const size_t s = rng();
					io.post([&, i, s]()
					{
						lig.monte_carlo(result_containers[i], s, sf, rec);
						cnt.increment();
					});
				}
				cnt.wait();

				// Merge results from all tasks into one single result container.
				assert(results.empty());
				const double required_square_error = static_cast<double>(4 * lig.num_heavy_atoms); // Ligands with RMSD < 2.0 will be clustered into the same cluster.
				for (auto& result_container : result_containers)
				{
					for (auto& result : result_container)
					{
						result::push(results, move(result), required_square_error);
					}
					result_container.clear();
				}

				num_confs = results.size();
				if (num_confs)
				{
					// Adjust free energy relative to the best conformation and flexibility.
					const auto& best_result = results.front();
					const double best_result_intra_e = best_result.e - best_result.f;
					for (auto& result : results)
					{
						result.e_nd = (result.e - best_result_intra_e) * lig.flexibility_penalty_factor;
						if (with_rf_score)
						{
							result.rf = lig.calculate_rf_score(result, rec, f);
						}
						// Result from compose_result is not complete and need to be completed.
						lig.calculate_by_comp(result, sf, rec, mask);
					}
					id_score = best_result.e_nd;
					rf_score = best_result.rf;
				}
			}

			// To run scoring against input ligand.
			if (score_only || both_score_dock)
			{
				++num_confs;
				if (precise_mode)
				{
					// The returned result is complete with per residue/heavy_atom energy.
					auto r0 = lig.complete_result_noconf(origin, sf, rec, mask);
					r0.e_nd = r0.f * lig.flexibility_penalty_factor;
					if (with_rf_score)
					{
						r0.rf = lig.calculate_rf_score(r0, rec, f);
					}
					id_score = r0.e_nd;
					rf_score = r0.rf;
					results.insert(results.begin(), move(r0));
				}
				else
				{
					conformation c0(lig.num_active_torsions);
					c0.position = origin;
					double e0, f0;
					change g0(0);
					lig.evaluate(c0, sf, rec, -99, e0, f0, g0);
					auto r0 = lig.compose_result(e0, f0, c0, false);
					r0.e_nd = r0.f * lig.flexibility_penalty_factor;
					if (with_rf_score)
					{
						r0.rf = lig.calculate_rf_score(r0, rec, f);
					}
					// Result from compose_result is not complete and need to be completed.
					lig.calculate_by_comp(r0, sf, rec, mask);
					id_score = r0.e_nd;
					rf_score = r0.rf;
					results.insert(results.begin(), move(r0));
				}
			}

			// If conformations are found, output them.
			if (num_confs)
			{
				// Write models to file.
				lig.write_models(output_ligand_path, results, rec);

				// Output per residue energy for all conformations.
				ofstream rep(out_path / (stem + ".csv"));
				rep.setf(ios::fixed, ios::floatfield);
				rep << "Chain ID,Residue name,Residue sequence";

				for (size_t i = 0; i < num_confs; ++i)
				{
					rep << ",Conf " << i + 1;
					if (!results[i].from_docking)
						rep << "(Input)";
				}
				rep << endl << setprecision(3);

				for (size_t k = 0; k < mask.size(); ++k)
				{
					if (!mask[k])
						continue;

					const auto& res = rec.residues[k];
					rep << res.chain << ',' << res.name << ',' << res.seq;

					for (size_t i = 0; i < num_confs; ++i)
					{
						rep << ',';
						if (results[i].e_residues[k] != 0.0)
							rep << results[i].e_residues[k];
					}

					rep << endl;
				}

				rep << endl;
				if (with_rf_score)
				{
					rep << "Binding Affinity,,";
					for (size_t i = 0; i < num_confs; ++i)
					{
						rep << ',' << results[i].rf;
					}
					rep << endl;
				}

				rep << "Intra-Ligand Free,,";
				for (size_t i = 0; i < num_confs; ++i)
				{
					rep << ',' << (results[i].e - results[i].f);
				}
				rep << endl;

				rep << "Inter-Ligand Free,,";
				for (size_t i = 0; i < num_confs; ++i)
				{
					rep << ',' << results[i].f;
				}
				rep << endl;

				rep << "Total Free Energy,,";
				for (size_t i = 0; i < num_confs; ++i)
				{
					rep << ',' << results[i].e;
				}
				rep << endl;

				rep << "Normalized Total Free Energy,,";
				for (size_t i = 0; i < num_confs; ++i)
				{
					rep << ',' << results[i].e_nd;
				}
				rep << endl;

				// Clear the results of the current ligand.
				results.clear();
			}
		}

		// If output file or conformations are found, output the idock score and RF-Score.
		cout << separator << setw(6) << num_confs;
		log << ',' << num_confs;
		if (num_confs)
		{
			cout << separator << setw(22) << id_score;
			log << ',' << id_score;
			if (with_rf_score)
			{
				cout << separator << setw(14) << rf_score;
				log << ',' << rf_score;
			}
		}
		cout << endl;
		log << endl;

		// Output to the log file in csv format. The log file can be sorted using: head -1 log.csv && tail -n +2 log.csv | awk -F, '{ printf "%s,%s\n", $2||0, $0 }' | sort -t, -k1nr -k6n | cut -d, -f2-
	}

	// Wait until the io service pool has finished all its tasks.
	io.wait();
}
