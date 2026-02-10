#include "MappingLoader.h"
#include "EpanetWrapper.h"
#include "json.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>

using json = nlohmann::json;

/**
 * @brief Load configuration from JSON file
 * 
 * Parses the JSON file and populates the MappingConfig structure.
 * Uses nlohmann/json library for JSON parsing.
 * 
 * Requirements: 2.1, 2.2
 * 
 * @param filename Path to JSON configuration file (e.g., "EpanetBridge.json")
 * @param config Output parameter to store loaded configuration
 * @param error Output parameter to store error message if loading fails
 * @return true if configuration was loaded successfully, false otherwise
 */
bool MappingLoader::LoadConfig(const std::string& filename, 
                               MappingConfig& config, 
                               std::string& error) {
    // Try to open the file
    std::ifstream file(filename);
    if (!file.is_open()) {
        error = "Configuration file '" + filename + "' not found in working directory";
        return false;
    }

    // Parse JSON
    json j;
    try {
        file >> j;
    }
    catch (const json::parse_error& e) {
        error = "Invalid JSON: " + std::string(e.what());
        return false;
    }

    // Extract required fields
    try {
        // Version (required)
        if (!j.contains("version")) {
            error = "Invalid JSON: missing required field 'version'";
            return false;
        }
        config.version = j["version"].get<std::string>();

        // Logging level (optional, defaults to "INFO")
        if (j.contains("logging_level")) {
            config.logging_level = j["logging_level"].get<std::string>();
        } else {
            config.logging_level = "INFO";
        }

        // INP file (optional)
        if (j.contains("inp_file")) {
            config.inp_file = j["inp_file"].get<std::string>();
        }

        // INP file hash (optional)
        if (j.contains("inp_file_hash")) {
            config.inp_file_hash = j["inp_file_hash"].get<std::string>();
        }

        // Hydraulic timestep (optional)
        if (j.contains("hydraulic_timestep")) {
            config.hydraulic_timestep = j["hydraulic_timestep"].get<int>();
        }

        // Input count (required)
        if (!j.contains("input_count")) {
            error = "Invalid JSON: missing required field 'input_count'";
            return false;
        }
        config.input_count = j["input_count"].get<int>();

        // Output count (required)
        if (!j.contains("output_count")) {
            error = "Invalid JSON: missing required field 'output_count'";
            return false;
        }
        config.output_count = j["output_count"].get<int>();

        // Inputs array (required)
        if (!j.contains("inputs")) {
            error = "Invalid JSON: missing required field 'inputs'";
            return false;
        }
        
        const json& inputs_array = j["inputs"];
        if (!inputs_array.is_array()) {
            error = "Invalid JSON: 'inputs' must be an array";
            return false;
        }

        config.inputs.clear();
        for (const auto& input_json : inputs_array) {
            InputMapping input;
            
            // Required fields for each input
            if (!input_json.contains("index")) {
                error = "Invalid JSON: input entry missing required field 'index'";
                return false;
            }
            input.index = input_json["index"].get<int>();

            if (!input_json.contains("name")) {
                error = "Invalid JSON: input entry missing required field 'name'";
                return false;
            }
            input.name = input_json["name"].get<std::string>();

            if (!input_json.contains("object_type")) {
                error = "Invalid JSON: input entry missing required field 'object_type'";
                return false;
            }
            input.object_type = input_json["object_type"].get<std::string>();

            if (!input_json.contains("property")) {
                error = "Invalid JSON: input entry missing required field 'property'";
                return false;
            }
            input.property = input_json["property"].get<std::string>();

            // Optional epanet_index (will be resolved later if not provided)
            if (input_json.contains("epanet_index")) {
                input.epanet_index = input_json["epanet_index"].get<int>();
            } else {
                input.epanet_index = 0;  // Not yet resolved
            }

            config.inputs.push_back(input);
        }

        // Outputs array (required)
        if (!j.contains("outputs")) {
            error = "Invalid JSON: missing required field 'outputs'";
            return false;
        }
        
        const json& outputs_array = j["outputs"];
        if (!outputs_array.is_array()) {
            error = "Invalid JSON: 'outputs' must be an array";
            return false;
        }

        config.outputs.clear();
        for (const auto& output_json : outputs_array) {
            OutputMapping output;
            
            // Required fields for each output
            if (!output_json.contains("index")) {
                error = "Invalid JSON: output entry missing required field 'index'";
                return false;
            }
            output.index = output_json["index"].get<int>();

            if (!output_json.contains("name")) {
                error = "Invalid JSON: output entry missing required field 'name'";
                return false;
            }
            output.name = output_json["name"].get<std::string>();

            if (!output_json.contains("object_type")) {
                error = "Invalid JSON: output entry missing required field 'object_type'";
                return false;
            }
            output.object_type = output_json["object_type"].get<std::string>();

            if (!output_json.contains("property")) {
                error = "Invalid JSON: output entry missing required field 'property'";
                return false;
            }
            output.property = output_json["property"].get<std::string>();

            // Optional epanet_index (will be resolved later if not provided)
            if (output_json.contains("epanet_index")) {
                output.epanet_index = output_json["epanet_index"].get<int>();
            } else {
                output.epanet_index = 0;  // Not yet resolved
            }

            config.outputs.push_back(output);
        }

    }
    catch (const json::type_error& e) {
        error = "Invalid JSON: type error - " + std::string(e.what());
        return false;
    }
    catch (const std::exception& e) {
        error = "Error parsing JSON: " + std::string(e.what());
        return false;
    }

    return true;
}

/**
 * @brief Validate configuration schema and data
 * 
 * Checks that:
 * - All required fields are present
 * - Field types are correct
 * - input_count matches inputs array length
 * - output_count matches outputs array length
 * - object_type values are valid
 * - property values are valid for each object_type
 * 
 * Requirements: 2.2, 2.4, 2.5
 * 
 * @param config Configuration to validate
 * @param error Output parameter to store error message if validation fails
 * @return true if configuration is valid, false otherwise
 */
bool MappingLoader::ValidateConfig(const MappingConfig& config, 
                                  std::string& error) {
    // Validate version format (should be X.Y)
    if (config.version.empty()) {
        error = "Invalid configuration: version is empty";
        return false;
    }

    // Validate logging level
    if (!config.logging_level.empty()) {
        std::string level = config.logging_level;
        if (level != "OFF" && level != "NONE" && level != "ERROR" && 
            level != "INFO" && level != "DEBUG") {
            error = "Invalid configuration: logging_level must be OFF, NONE, ERROR, INFO, or DEBUG";
            return false;
        }
    }

    // Validate input_count matches inputs array length
    if (config.input_count != static_cast<int>(config.inputs.size())) {
        std::ostringstream oss;
        oss << "Argument count mismatch: JSON specifies " << config.input_count 
            << " inputs but array contains " << config.inputs.size() << " entries";
        error = oss.str();
        return false;
    }

    // Validate output_count matches outputs array length
    if (config.output_count != static_cast<int>(config.outputs.size())) {
        std::ostringstream oss;
        oss << "Argument count mismatch: JSON specifies " << config.output_count 
            << " outputs but array contains " << config.outputs.size() << " entries";
        error = oss.str();
        return false;
    }

    // Validate each input mapping
    for (size_t i = 0; i < config.inputs.size(); ++i) {
        const InputMapping& input = config.inputs[i];

        // Validate index is non-negative
        if (input.index < 0) {
            std::ostringstream oss;
            oss << "Invalid input[" << i << "]: index must be non-negative";
            error = oss.str();
            return false;
        }

        // Validate name is not empty
        if (input.name.empty()) {
            std::ostringstream oss;
            oss << "Invalid input[" << i << "]: name is empty";
            error = oss.str();
            return false;
        }

        // Validate object_type
        if (!IsValidObjectType(input.object_type, true)) {
            std::ostringstream oss;
            oss << "Invalid input[" << i << "]: object_type '" << input.object_type 
                << "' must be SYSTEM, NODE, LINK, or PATTERN";
            error = oss.str();
            return false;
        }

        // Validate property for this object_type
        if (!IsValidProperty(input.object_type, input.property, true)) {
            std::ostringstream oss;
            oss << "Invalid input[" << i << "]: property '" << input.property 
                << "' is not valid for object_type '" << input.object_type << "'";
            error = oss.str();
            return false;
        }
    }

    // Validate each output mapping
    for (size_t i = 0; i < config.outputs.size(); ++i) {
        const OutputMapping& output = config.outputs[i];

        // Validate index is non-negative
        if (output.index < 0) {
            std::ostringstream oss;
            oss << "Invalid output[" << i << "]: index must be non-negative";
            error = oss.str();
            return false;
        }

        // Validate name is not empty
        if (output.name.empty()) {
            std::ostringstream oss;
            oss << "Invalid output[" << i << "]: name is empty";
            error = oss.str();
            return false;
        }

        // Validate object_type
        if (!IsValidObjectType(output.object_type, false)) {
            std::ostringstream oss;
            oss << "Invalid output[" << i << "]: object_type '" << output.object_type 
                << "' must be NODE or LINK";
            error = oss.str();
            return false;
        }

        // Validate property for this object_type
        if (!IsValidProperty(output.object_type, output.property, false)) {
            std::ostringstream oss;
            oss << "Invalid output[" << i << "]: property '" << output.property 
                << "' is not valid for object_type '" << output.object_type << "'";
            error = oss.str();
            return false;
        }
    }

    return true;
}

/**
 * @brief Resolve element names to EPANET internal indices
 * 
 * For each input and output mapping, this method:
 * - Calls ENgetnodeindex() for NODE object types
 * - Calls ENgetlinkindex() for LINK object types
 * - Stores the resolved index in the epanet_index field
 * - Returns error if any element name cannot be resolved
 * 
 * Note: This method requires EPANET to be already opened via ENopen().
 * 
 * Requirements: 2.1
 * 
 * @param config Configuration with element names to resolve (modified in place)
 * @param error Output parameter to store error message if resolution fails
 * @return true if all indices were resolved successfully, false otherwise
 */
bool MappingLoader::ResolveIndices(MappingConfig& config, 
                                  std::string& error) {
    // Resolve indices for all input mappings
    for (size_t i = 0; i < config.inputs.size(); ++i) {
        InputMapping& input = config.inputs[i];
        
        // Skip SYSTEM and PATTERN types - they don't have EPANET indices
        if (input.object_type == "SYSTEM" || input.object_type == "PATTERN") {
            input.epanet_index = 0;  // Not applicable
            continue;
        }
        
        // Skip if index is already resolved (pre-populated in JSON)
        if (input.epanet_index > 0) {
            continue;
        }
        
        // Resolve NODE indices
        if (input.object_type == "NODE") {
            int index = 0;
            int errorCode = EpanetWrapper::GetNodeIndex(input.name, &index);
            
            if (errorCode != 0) {
                std::ostringstream oss;
                oss << "Element '" << input.name << "' not found in EPANET model (error code " 
                    << errorCode << ")";
                error = oss.str();
                return false;
            }
            
            input.epanet_index = index;
        }
        // Resolve LINK indices
        else if (input.object_type == "LINK") {
            int index = 0;
            int errorCode = EpanetWrapper::GetLinkIndex(input.name, &index);
            
            if (errorCode != 0) {
                std::ostringstream oss;
                oss << "Element '" << input.name << "' not found in EPANET model (error code " 
                    << errorCode << ")";
                error = oss.str();
                return false;
            }
            
            input.epanet_index = index;
        }
    }
    
    // Resolve indices for all output mappings
    for (size_t i = 0; i < config.outputs.size(); ++i) {
        OutputMapping& output = config.outputs[i];
        
        // Skip if index is already resolved (pre-populated in JSON)
        if (output.epanet_index > 0) {
            continue;
        }
        
        // Resolve NODE indices
        if (output.object_type == "NODE") {
            int index = 0;
            int errorCode = EpanetWrapper::GetNodeIndex(output.name, &index);
            
            if (errorCode != 0) {
                std::ostringstream oss;
                oss << "Element '" << output.name << "' not found in EPANET model (error code " 
                    << errorCode << ")";
                error = oss.str();
                return false;
            }
            
            output.epanet_index = index;
        }
        // Resolve LINK indices
        else if (output.object_type == "LINK") {
            int index = 0;
            int errorCode = EpanetWrapper::GetLinkIndex(output.name, &index);
            
            if (errorCode != 0) {
                std::ostringstream oss;
                oss << "Element '" << output.name << "' not found in EPANET model (error code " 
                    << errorCode << ")";
                error = oss.str();
                return false;
            }
            
            output.epanet_index = index;
        }
    }
    
    return true;
}

/**
 * @brief Validate that object_type is one of the allowed values
 * 
 * For inputs: "SYSTEM", "NODE", "LINK", "PATTERN"
 * For outputs: "NODE", "LINK"
 * 
 * @param object_type The object type string to validate
 * @param is_input True if validating an input, false for output
 * @return true if object_type is valid, false otherwise
 */
bool MappingLoader::IsValidObjectType(const std::string& object_type, bool is_input) {
    if (is_input) {
        // Valid input object types
        return (object_type == "SYSTEM" || 
                object_type == "NODE" || 
                object_type == "LINK" || 
                object_type == "PATTERN");
    } else {
        // Valid output object types
        return (object_type == "NODE" || 
                object_type == "LINK");
    }
}

/**
 * @brief Validate that property is valid for the given object_type
 * 
 * Valid combinations are defined in the design document.
 * Examples:
 * - NODE: "DEMAND", "PRESSURE", "HEAD", "TANKLEVEL", "QUALITY"
 * - LINK: "STATUS", "SETTING", "FLOW", "VELOCITY", "HEADLOSS"
 * - PATTERN: "MULTIPLIER"
 * - SYSTEM: "ELAPSEDTIME"
 * 
 * @param object_type The object type
 * @param property The property name
 * @param is_input True if validating an input, false for output
 * @return true if property is valid for the object_type, false otherwise
 */
bool MappingLoader::IsValidProperty(const std::string& object_type, 
                                   const std::string& property, 
                                   bool is_input) {
    if (object_type == "SYSTEM") {
        // SYSTEM only valid for inputs
        if (!is_input) return false;
        return (property == "ELAPSEDTIME");
    }
    else if (object_type == "NODE") {
        if (is_input) {
            // Valid NODE input properties
            return (property == "DEMAND" || 
                    property == "BASEDEMAND" ||
                    property == "ELEVATION" ||
                    property == "EMITTER" ||
                    property == "INITQUAL" ||
                    property == "SOURCEQUAL" ||
                    property == "SOURCEPAT" ||
                    property == "SOURCETYPE");
        } else {
            // Valid NODE output properties
            return (property == "DEMAND" || 
                    property == "HEAD" || 
                    property == "PRESSURE" || 
                    property == "TANKLEVEL" ||
                    property == "QUALITY");
        }
    }
    else if (object_type == "LINK") {
        if (is_input) {
            // Valid LINK input properties
            return (property == "STATUS" || 
                    property == "SETTING" ||
                    property == "DIAMETER" ||
                    property == "LENGTH" ||
                    property == "ROUGHNESS" ||
                    property == "MINORLOSS" ||
                    property == "INITSTATUS" ||
                    property == "INITSETTING" ||
                    property == "KBULK" ||
                    property == "KWALL");
        } else {
            // Valid LINK output properties
            return (property == "FLOW" || 
                    property == "VELOCITY" || 
                    property == "HEADLOSS" || 
                    property == "STATUS" ||
                    property == "SETTING" ||
                    property == "ENERGY" ||
                    property == "QUALITY");
        }
    }
    else if (object_type == "PATTERN") {
        // PATTERN only valid for inputs
        if (!is_input) return false;
        return (property == "MULTIPLIER");
    }

    return false;
}
