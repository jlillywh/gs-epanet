#pragma once

#include <string>
#include <vector>

/**
 * @brief Input mapping structure defining how GoldSim inputs map to EPANET parameters
 * 
 * Each input mapping specifies:
 * - index: Position in the GoldSim inargs array
 * - name: Element name in EPANET model (e.g., "JUNCTION_1", "PUMP_1")
 * - object_type: Type of EPANET object ("SYSTEM", "NODE", "LINK", "PATTERN")
 * - property: Property to modify (e.g., "DEMAND", "STATUS", "SETTING")
 * - epanet_index: Resolved EPANET internal index (set during initialization)
 * 
 * Requirements: 2.1, 2.5
 */
struct InputMapping {
    int index;                  ///< Index in GoldSim inargs array
    std::string name;           ///< Element name in EPANET model
    std::string object_type;    ///< Object type: "SYSTEM", "NODE", "LINK", "PATTERN"
    std::string property;       ///< Property name: "DEMAND", "STATUS", "SETTING", etc.
    int epanet_index;           ///< Resolved EPANET index (0 if not yet resolved)

    /**
     * @brief Construct a new Input Mapping object with default values
     */
    InputMapping() : index(0), epanet_index(0) {}
};

/**
 * @brief Output mapping structure defining how EPANET results map to GoldSim outputs
 * 
 * Each output mapping specifies:
 * - index: Position in the GoldSim outargs array
 * - name: Element name in EPANET model (e.g., "JUNCTION_1", "PIPE_1")
 * - object_type: Type of EPANET object ("NODE", "LINK")
 * - property: Property to extract (e.g., "PRESSURE", "FLOW", "TANKLEVEL")
 * - epanet_index: Resolved EPANET internal index (set during initialization)
 * 
 * Requirements: 2.1, 2.5
 */
struct OutputMapping {
    int index;                  ///< Index in GoldSim outargs array
    std::string name;           ///< Element name in EPANET model
    std::string object_type;    ///< Object type: "NODE", "LINK"
    std::string property;       ///< Property name: "PRESSURE", "FLOW", "HEAD", etc.
    int epanet_index;           ///< Resolved EPANET index (0 if not yet resolved)

    /**
     * @brief Construct a new Output Mapping object with default values
     */
    OutputMapping() : index(0), epanet_index(0) {}
};

/**
 * @brief Configuration structure containing all mapping information from JSON
 * 
 * This structure holds the complete configuration loaded from EpanetBridge.json,
 * including metadata, logging settings, and input/output mappings.
 * 
 * Requirements: 2.1, 2.2
 */
struct MappingConfig {
    std::string version;            ///< Configuration version (e.g., "1.0")
    std::string logging_level;      ///< Log level: "OFF", "NONE", "ERROR", "INFO", "DEBUG"
    std::string inp_file;           ///< Path to EPANET .inp file
    std::string inp_file_hash;      ///< MD5 hash of .inp file for validation
    int hydraulic_timestep;         ///< EPANET hydraulic timestep in seconds
    int input_count;                ///< Number of inputs (including ElapsedTime)
    int output_count;               ///< Number of outputs
    std::vector<InputMapping> inputs;   ///< Array of input mappings
    std::vector<OutputMapping> outputs; ///< Array of output mappings

    /**
     * @brief Construct a new Mapping Config object with default values
     */
    MappingConfig() : hydraulic_timestep(0), input_count(0), output_count(0) {}
};

/**
 * @brief MappingLoader class for loading and validating JSON configuration
 * 
 * The MappingLoader class provides static methods for:
 * - Loading JSON configuration from file
 * - Validating configuration schema and data
 * - Resolving element names to EPANET internal indices
 * 
 * Requirements: 2.1, 2.2, 2.4, 2.5
 */
class MappingLoader {
public:
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
    static bool LoadConfig(const std::string& filename, 
                          MappingConfig& config, 
                          std::string& error);

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
    static bool ValidateConfig(const MappingConfig& config, 
                              std::string& error);

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
     * It will be completed after EpanetWrapper is implemented.
     * 
     * Requirements: 2.1
     * 
     * @param config Configuration with element names to resolve (modified in place)
     * @param error Output parameter to store error message if resolution fails
     * @return true if all indices were resolved successfully, false otherwise
     */
    static bool ResolveIndices(MappingConfig& config, 
                              std::string& error);

private:
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
    static bool IsValidObjectType(const std::string& object_type, bool is_input);

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
    static bool IsValidProperty(const std::string& object_type, 
                               const std::string& property, 
                               bool is_input);
};
