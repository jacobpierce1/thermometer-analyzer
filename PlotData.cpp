#include <iostream>
#include <fstream>
#include <stdio.h>
#include <vector>
#include <stdlib.h>
using namespace std;

// struct to store all the information for a plot 
struct plot_args {
  int mode;
  int num_channels;
  vector<string> projects;
  vector<int> channels; 
  vector<float> time_shifts;
  vector<string> leg_labels;
  string title;
  float time_cut;
  bool fill_tree;
};

// functions
void make_project(plot_args args);
void make_compare(plot_args args);
bool file_exists(string proj_name);
void graph_project(plot_args args, bool adjust_time);
void graph_compare(plot_args args, bool adjust_time);
TLegend *make_legend ();
void adjust_t0_project(plot_args args);
void adjust_t0_compare(plot_args args);
void copy_files (plot_args args, int i);
vector<string> split(string str, char delimiter);
void fill_tree(plot_args args);
string switch_chars(string str, char start_char, char end_char);
void combine_temp_files(plot_args args);
const string currentDateTime();
void f(); 

// constants
const char script_delimiter = '!';
const string dick = "dick";


/////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////



// function to make a single project
void make_project(plot_args args) {
  cout << "Generating project" << endl;
  // make sure file exists
  if(!file_exists(args.projects[0])) {
    cout << "Project does not exist ";
    return;
  }

  // adjust data if necessary
  bool adjust_time = !(args.time_shifts[0]==0 && args.time_cut==0);
  if (adjust_time) adjust_t0_project(args);

  // make the graph
  graph_project(args, adjust_time);
}



// function to compare two projects
void make_compare(plot_args args) {
  cout << "Generating comparison" << endl;
  
  // make sure all files exist
  for (int i=0; i<args.num_channels; i++) {
    if (!file_exists(args.projects[i])) {
      cout << "Project " << i << " does not exist: "; 
      return;
    }
  }

  // perform time shift if necessary
  bool adjust_time = false;
  for (int i=0; i<args.num_channels; i++) if (args.time_shifts[i] != 0 
					      || args.time_cut != 0) adjust_time = true;
  if (adjust_time) adjust_t0_compare(args);

  // make the graph
  graph_compare(args, adjust_time); 
}



// make graphs for the project
void graph_project(plot_args args, bool adjust_time) {

  // declare canvas, graphs, multigraph, legend
  TCanvas *c1 = new TCanvas("c1", "Thermal Profile", 200, 10, 700, 500);
  TGraph *graphs[args.num_channels];
  TMultiGraph *mg = new TMultiGraph();
  string mg_title = args.title + ";" + "Time elapsed (min); Temperature (C)";
  mg->SetTitle(mg_title.c_str());

  // make legend
  TLegend *leg = make_legend();

  // make graphs and add to multigraph
  string directory = "./THERM_TESTS/" + args.projects[0] + "/" + args.projects[0] + "_ch";
  for(int i=0; i<args.num_channels; i++) {

    // file to open: time_adjusted or raw
    string file;
    if (args.time_shifts[0]==0 && args.time_cut==0) {
      file = directory + to_string(i) + ".txt";
      cout << file << endl;
    }
    else {
      file = "./THERM_TESTS/TEMP/temp_ch" + to_string(i) + ".txt";
    }
    // make graph from the obtained file
    graphs[i] = new TGraph(file.c_str());
    graphs[i]->SetLineWidth(2);
    graphs[i]->SetMarkerColorAlpha(i+1, 0);
    graphs[i]->SetLineColor(i+1);
    graphs[i]->SetFillColor(0);

    //add to multigraph and legend
    mg->Add(graphs[i]);
    leg->AddEntry(graphs[i], (args.leg_labels[i]).c_str()); 
  }

  // draw multigraph and legend; return.
  mg->Draw("ACP");
  leg->Draw();
  return;
}



// make graphs for the project
void graph_compare(plot_args args, bool adjust_time) {
  // declare canvas, graphs, multigraph, legend
  TCanvas *c1 = new TCanvas("c1", "Thermal Profile", 200, 10, 700, 500);
  TGraph *graphs[args.num_channels];
  TMultiGraph *mg = new TMultiGraph();
  string mg_title = args.title + ";" + "Time elapsed (min); Temperature (C)";
  mg->SetTitle(mg_title.c_str());

  // make legend
  TLegend *leg = make_legend();

    // make graphs and add to multigraph
  for(int i=0; i<args.num_channels; i++) {
    // file to open: time_adjusted or raw
    string file;
    if (!adjust_time) {
      string directory = "./THERM_TESTS/" + args.projects[i] + "/" + args.projects[i] + "_ch";
      file = directory + to_string(args.channels[i]) + ".txt";
      cout << file << endl;
    }
    else file = "./THERM_TESTS/TEMP/temp_ch" + to_string(i) + ".txt";

    // create graph from txt file
    graphs[i] = new TGraph(file.c_str());
    graphs[i]->SetLineWidth(2);
    graphs[i]->SetMarkerColorAlpha(i+1, 0);
    graphs[i]->SetLineColor(i+1);  // lucky coincidence that these are the colors we want
    graphs[i]->SetFillColor(0);

    //add to multigraph and legend
    mg->Add(graphs[i]);
    leg->AddEntry(graphs[i], args.leg_labels[i].c_str());
  }
  // draw multigraph and legend; return.
  mg->Draw("ACP");
  leg->Draw();
  return;
}



// construct the legend
TLegend *make_legend() { 
  leg_w = .2;
  leg_h = .15;
  leg_x = .67;
  leg_y = .71;
  TLegend *leg = new TLegend(leg_x, leg_y, leg_x+leg_w, leg_y+leg_h);
  leg->SetHeader("Legend"); 
  return leg;
}



// check whether file exists
bool file_exists(string proj_name) {
  string directory = "./THERM_TESTS/" + proj_name + "/" + proj_name + "_ch0.txt";
  ifstream test_file(directory.c_str());
  return test_file.good();
}



// for single project: set t=0
void adjust_t0_project (plot_args args) {
  // duplicate files
  for (int i=0; i<args.num_channels; i++) copy_files(args, i);
  return;
}



// for comparison: set t=0 of plot 1, move other plots rel to plot 1
void adjust_t0_compare (plot_args args) {
  // duplicate files
  for (int i=0; i<args.num_channels; i++) copy_files(args, i);
  return;
}



// duplicate files
void copy_files (plot_args args, int i) {
  
  // construct strings we will use 
  string project = "";
  if (args.mode == 0) project = args.projects[0];
  else project = args.projects[i];

  string raw_file_directory = "/home/helix/thermo_test/THERM_TESTS/" + project + "/" 
    + project + "_ch" + to_string(args.channels[i]) + ".txt";

  string temp_file_directory = "/home/helix/thermo_test/THERM_TESTS/TEMP/temp_ch" 
    + to_string(i) + ".txt";

  ifstream raw_file;
  ofstream temp_file;
  
  // open files
  raw_file.open(raw_file_directory.c_str());
  temp_file.open(temp_file_directory.c_str(), std::ofstream::trunc);  // opens as empty

  // read all data from raw_files[i] into temp_files[i]
  string line;
  while(getline(raw_file, line)) {

    // get line and modify time
    int space_pos = line.find(" ");
    string time = line.substr(0, space_pos);
    string temp = line.substr(space_pos + 1);
    float time_float = atof(time.c_str());

    // handle differently for plot 0 in compare mode
    if (args.mode == 1 && i != 0) time_float -= args.time_shifts[0];
    time_float -= args.time_shifts[i];

    // create string and add to file if time is positive
    string temp_file_input = to_string(time_float) + " " + temp + "\n";
    if (time_float >= 0 && (time_float <= args.time_cut || args.time_cut == 0)) temp_file << temp_file_input;
  }
    
  // close files when finished
  raw_file.close();
  temp_file.close();
}



/* fill a ttree with the data and store in a binary file. This is not done in 
   helixgui.cpp because it is easiest to keep all the root things together. 
   In order to process in a mode-independent way, all the data we are using is stored 
   in advance in the TEMP directory */
void fill_tree(plot_args args) {
  cout << "Filling tree" << endl;

  // get title of the file (remove ambiguity for comparison)
  string title = "";
  if (args.title != "Comparison") title = args.title;
  else title = args.title + "_" + currentDateTime();

  // open file and use correct directory
  string file_name = title + ".root";
  string file_location = "/home/helix/thermo_test/THERM_TESTS/";
  TFile ofile(file_name.c_str(), "RECREATE");
  
  // declare tree and branches
  TTree *tree = new TTree("name", "title");   // todo
  float time;
  tree->Branch("Time", &time, "Time/F");

  float temps[args.num_channels];
  for (int i=0; i<args.num_channels; i++) {
    string branch_name = "Temps" + to_string(i);
    tree->Branch(branch_name.c_str(), &temps[i], (branch_name + "/F").c_str());
  }

  // open the files to be read
  ifstream files[5];
  bool adjust_time = false;
  for (int i=0; i<args.num_channels; i++) if (args.time_shifts[i] != 0 
					     || args.time_cut != 0) adjust_time = true;
  
  // if time is adjusted, we will be in the ./TEMP/ directory for either mode 
  if (adjust_time) {
    for (int i=0; i<args.num_channels; i++) {
      files[i].open((file_location + "TEMP/temp_ch" + to_string(i)).c_str());
    }
  }

  else if (args.mode == 0) {
    for (int i=0; i<args.num_channels; i++) {
      string temp = file_location + args.projects[0] + "/" + args.projects[0] + "_ch" + to_string(i) + ".txt";
      cout << temp << endl;
      files[i].open(temp.c_str());
    }
  }


  else if (args.mode == 1) {
    for (int i=0; i<args.num_channels; i++) {
      files[i].open((file_location + args.projects[i] + "/" + args.projects[i] + "_ch" + to_string(args.channels[i]) + ".txt").c_str());
    }
  }
  
  // open the files and read into the them
  string line;
  for (int i=0; i<args.num_channels; i++) {
    while(getline(files[i], line)) {

      // get data
      vector<string> data = split(line, ' ');
      time = atof(data[0].c_str());
      temps[i] = atof(data[1].c_str());

      // add time if not there already

      // write into the tree
      int status = tree->Fill();
    }
  }

  // save the tree to the file and close the file.
  tree->Write();
  ofile.Close();

  /* by now, there should be a file in the directory of PlotData.cpp 
     called <project>.root. this manually moves the file to the 
     correct directory. there is probably a better way to do this. */
  string sys_cmd_tmp = "mv " + file_name + " " + "/home/helix/thermo_test/TTREES/";
  const char *sys_cmd = sys_cmd_tmp.c_str();
  system(sys_cmd);
  return;
}



// split according to delimiter
vector<string> split(string str, char delimiter) {
  // initializing vars
  size_t length = count(str.begin(), str.end(), delimiter) + 1;
  vector<string> split_string(length);
  size_t pos = 0;
  string token = "";

  // loop through str
  int i = 0;
  while ((pos = str.find(delimiter)) != std::string::npos) {
    token = str.substr(0, pos);
    str.erase(0, pos + 1);
    split_string[i] = token;
    i++;
  }
  if (str != "") split_string[i] = str;

  return split_string;
}



string replace_chars(string str, char start_char, char end_char) {
  for(string::iterator it = str.begin(); it != str.end(); it++) {
    if(*it == start_char) *it = end_char;
  }
  return str;
}


void f() {
  cout << dick << endl; 
}



// parse args from command line and return as struct plot_args. borrowed from stackexchange.
plot_args parse_args(string arg_string) {
  
  // initializing everything
  vector<string> parsed_args = split(arg_string, script_delimiter);
  int num_args = parsed_args.size() - 1;
  parsed_args.erase(parsed_args.begin() + num_args);       // fixes weird parsing error

  // fill args, starting with the mode
  plot_args args;
  args.num_channels = count(parsed_args[0].begin(), parsed_args[0].end(), ',') + 1;
  if (args.num_channels == 1) args.mode = 0;
  else args.mode = 1;

  // args must be parsed differently for different modes
  // mode 0: single project
  if(args.mode == 0) {

    // handle project name and num channels
    string temp_project = parsed_args[0];
    size_t len = temp_project.length();
    if (temp_project[len-2] == '_') {
	args.num_channels = atoi(temp_project.substr(len-1, 1).c_str());
	args.projects.push_back(temp_project.substr(0, len-2));  
    }
    else {
      args.num_channels = 4;   //todo
      args.projects.push_back(temp_project);
    }

    // set default channels
    args.channels = {0, 1, 2, 3};
    
    // time shift
    if (num_args >= 4) for (int i=0; i<args.num_channels; i++) args.time_shifts.push_back(atof(parsed_args[3].c_str()));
    else for (int i=0; i<args.num_channels; i++) args.time_shifts.push_back(0);
  }
  
  // mode 1: compare multiple projects
  else if (args.mode == 1) {

    // get vectors for projects and channel numbers
    vector<string> temp_projects = split(parsed_args[0], ',');
    for (int i=0; i<args.num_channels; i++) {
      size_t len = temp_projects[i].length();
      args.channels.push_back(atoi(temp_projects[i].substr(len-1, 1).c_str()));
      args.projects.push_back(temp_projects[i].substr(0, len-1));
    }

    // handle time shifts
    if (num_args >= 4) {
      vector<string> temp_time_shifts = split(parsed_args[3], ',');
      for (int i=0; i<args.num_channels; i++) args.time_shifts.push_back(atof(temp_time_shifts[i].c_str()));
    } else for (int i=0; i<args.num_channels; i++) args.time_shifts.push_back(0);
  } 
  
  //default: mode not recognized
  else cout << "Mode not recognized, quitting " << endl;
  
  // mode-independent parsing

  // title
  if (num_args >= 3) {
    args.title = replace_chars(parsed_args[2], '_', ' ');
    if(args.title == "d" || args.title == "default") {
      if (args.mode == 0) args.title = args.projects[0];
      else args.title = "Comparison";
    }
  } else {
    if (args.mode == 0) args.title = args.projects[0];
    else args.title = "Comparison";
  }

  // get time cut
  if (num_args >= 5) args.time_cut = atof(parsed_args[4].c_str());
  else args.time_cut = 0;


  // legend labels. this is done last since it changes the mode.
  if (num_args >= 2){

    // check for the fill_tree mode
    if(parsed_args[1] == "tree") args.fill_tree = true;

    // otherwise proceed as normally
    else {
      args.leg_labels = split(parsed_args[1], ',');
      for (int i=0; i<args.num_channels; i++) {
	args.leg_labels[i] = replace_chars(args.leg_labels[i], '_', ' ');
	if (args.leg_labels[i] == "" || args.leg_labels[i] == "default" || 
	    args.leg_labels[i] == "d") args.leg_labels[i] = "Ch " + to_string(i);
      }
    }
  } else for (int i=0; i<args.num_channels; i++) args.leg_labels.push_back("Ch " + to_string(i));


  return args;
}



// get current date/time and report it as a string
const string currentDateTime() {
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%d-%T", &tstruct);

    return buf;
}



// main function
void PlotData(string arg_string) {

  // get a struct plot_args from the command line arg_string (see shell script)
  plot_args args = parse_args(arg_string);
  if (args.fill_tree) fill_tree(args);
  else { 
    if(args.mode == 0) make_project(args);
    else if(args.mode == 1) make_compare(args);
    else cout << "Mode not recognized, quitting" << endl;
  }
  return;
}
