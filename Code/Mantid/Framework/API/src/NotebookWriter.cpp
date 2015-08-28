#include "MantidAPI/NotebookWriter.h"
#include "MantidKernel/Logger.h"
#include "MantidKernel/MantidVersion.h"


namespace {
    Mantid::Kernel::Logger g_log("NotebookWriter");
}

NotebookWriter::NotebookWriter() : m_cell_buffer(Json::arrayValue)
{
  // Add header comments and code
  headerComment();
  headerCode();
}

void NotebookWriter::codeCell(Json::Value string_code) {

  Json::Value cell_data;
  const Json::Value empty = Json::Value(Json::ValueType::objectValue);

  cell_data["cell_type"] = "code";
  cell_data["collapsed"] = false;
  cell_data["input"] = string_code;
  cell_data["language"] = "python";
  cell_data["metadata"] = empty;
  cell_data["outputs"] = Json::Value(Json::arrayValue);

  m_cell_buffer.append(cell_data);
}

void NotebookWriter::codeCell(std::string string_code) {

  Json::Value cell_data;
  const Json::Value empty = Json::Value(Json::ValueType::objectValue);

  cell_data["cell_type"] = "code";
  cell_data["collapsed"] = false;
  cell_data["input"] = string_code;
  cell_data["language"] = "python";
  cell_data["metadata"] = empty;
  cell_data["outputs"] = Json::Value(Json::arrayValue);

  m_cell_buffer.append(cell_data);
}

void NotebookWriter::markdownCell(Json::Value string_array) {

  Json::Value cell_data;
  const Json::Value empty = Json::Value(Json::ValueType::objectValue);

  cell_data["cell_type"] = "markdown";
  cell_data["metadata"] = empty;
  cell_data["source"] = string_array;

  m_cell_buffer.append(cell_data);
}

void NotebookWriter::markdownCell(std::string string_array) {

  Json::Value cell_data;
  const Json::Value empty = Json::Value(Json::ValueType::objectValue);

  cell_data["cell_type"] = "markdown";
  cell_data["metadata"] = empty;
  cell_data["source"] = string_array;

  m_cell_buffer.append(cell_data);
}

void NotebookWriter::headerComment() {

  Json::Value strings(Json::arrayValue);
  strings.append(Json::Value("This IPython Notebook was automatically generated by MantidPlot, version: "));
  strings.append(Json::Value(Mantid::Kernel::MantidVersion::version()));
  strings.append(Json::Value("\n"));
  strings.append(Json::Value(Mantid::Kernel::MantidVersion::releaseNotes()));
  strings.append(Json::Value("\n\nThe following information may be useful:\n"
                                 "* [Mantid Framework Python API Reference]"
                                 "(http://docs.mantidproject.org/nightly/api/python/index.html)\n"
                                 "* [IPython Notebook Documentation](http://ipython.org/ipython-doc/stable/notebook/)\n"
                                 "* [matplotlib Documentation](http://matplotlib.org/contents.html)\n\n"
                                 "Help requests and bug reports should be submitted to the [Mantid forum.]"
                                 "(http://forum.mantidproject.org)"));

  markdownCell(strings);
}

void NotebookWriter::headerCode() {

  Json::Value import_mantid(Json::arrayValue);

  import_mantid.append(Json::Value("import sys\n"));
  import_mantid.append(Json::Value("import os\n\n"));

  import_mantid.append(Json::Value("#Tell python where Mantid is installed.\n"));
  import_mantid.append(Json::Value("#The official packages put this information in an environment variable called \"MANTIDPATH\"\n"));
  import_mantid.append(Json::Value("sys.path.append(os.environ['MANTIDPATH'])\n\n"));

  import_mantid.append(Json::Value("#We can now import Mantid's Python API\n"));
  import_mantid.append(Json::Value("from mantid.simpleapi import *"));

  codeCell(import_mantid);

  Json::Value check_version(Json::arrayValue);

  check_version.append(Json::Value("import warnings"));
  check_version.append(Json::Value("import mantid.kernel"));
  check_version.append(Json::Value("# Check if the version of Mantid being used matches"
                                       " the version which created the notebook."));
  check_version.append(Json::Value(std::string("if \"") + Mantid::Kernel::MantidVersion::version() +
                                   "\" != mantid.kernel.version_str(): warnings.warn(\"Version of Mantid"
                                       " being used does not match version which created the notebook.\")"));

  codeCell(check_version);

  Json::Value import_matplotlib(Json::arrayValue);

  import_matplotlib.append(Json::Value("#Import matplotlib's pyplot interface under the name 'plt'\n"));
  import_matplotlib.append(Json::Value("import matplotlib.pyplot as plt\n\n"));

  import_matplotlib.append(Json::Value("#Some magic to tell matplotlib how to behave in IPython Notebook\n"));
  import_matplotlib.append(Json::Value("%matplotlib inline")); //TODO nbagg available on all platforms?

  codeCell(import_matplotlib);
}

Json::Value NotebookWriter::buildNotebook() {

  Json::Value output;
  const Json::Value empty = Json::Value(Json::ValueType::objectValue);

  Json::Value worksheet;
  worksheet["cells"] = m_cell_buffer;
  worksheet["metadata"] = empty;

  Json::Value worksheet_arr(Json::arrayValue);
  worksheet_arr.append(worksheet);

  Json::Value meta_name;
  meta_name["name"] = "example"; //TODO create useful name
  output["metadata"] = meta_name;
  output["nbformat"] = 3;
  output["nbformat_minor"] = 0;
  output["worksheets"] = worksheet_arr;

  return output;
}

std::string NotebookWriter::writeNotebook() {

  const Json::Value root = buildNotebook();

  Json::StyledWriter writer;
  std::string output_string = writer.write(root);

  return output_string;
}