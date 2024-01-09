/*****************************************************************************/
 
/* Master 0, Slave 0, "MADHT1505BA1"
 * Vendor ID:       0x0000066f
 * Product code:    0x515050a1
 * Revision number: 0x00010000
 */

// process data

const int MADHT1505BA1_vendor          = 0x0000066f;
const int MADHT1505BA1_product_code    = 0x515050a1;

// offsets for PDO entries

typedef struct  {
	unsigned int control_word;
	unsigned int modes_of_operation;
	unsigned int target_position;
	unsigned int touch_probe_function;
	unsigned int error_code;
	unsigned int status_word;
	unsigned int modes_of_operation_display;
	unsigned int position_actual_value;
	unsigned int touch_probe_status;
	unsigned int touch_probe_pos1_pos_value;
	unsigned int following_error_actual_value;
	unsigned int digital_inputs;
	unsigned int current_velocity;
	unsigned int target_velocity;
	
	unsigned int alias;
	unsigned int position;

	ec_slave_config_t *sc;
	ec_slave_config_state_t sc_state;
	ec_domain_t *domain;
	ec_domain_state_t domain_state;
	uint8_t *domain_pd;
	ec_pdo_entry_reg_t domain_regs[15];

    unsigned int cpu_core;
    pthread_t *thread;

}MADHT1505BA1_object;

ec_pdo_entry_info_t slave_0_pdo_entries[] = {
    {0x6040, 0x00, 16}, /* Controlword */
    {0x6060, 0x00, 8}, /* Modes of operation */
    {0x607a, 0x00, 32}, /* Target position */
    {0x60b8, 0x00, 16}, /* Touch probe function */
    {0x60ff, 0x00, 32}, /* target_velocity */

    {0x603f, 0x00, 16}, /* Error code */
    {0x6041, 0x00, 16}, /* Statusword */
    {0x6061, 0x00, 8}, /* Modes of operation display */
    {0x6064, 0x00, 32}, /* Position actual value */
    {0x60b9, 0x00, 16}, /* Touch probe status */
    {0x60ba, 0x00, 32}, /* Touch probe pos1 pos value */
    {0x60f4, 0x00, 32}, /* Following error actual value */
    {0x60fd, 0x00, 32}, /* Digital inputs */
    {0x606c, 0x00, 32}, /* current_velocity */
};

ec_pdo_info_t slave_0_pdos[] = {
    {0x1600, 5, slave_0_pdo_entries + 0}, /* Receive PDO mapping 1 */
    {0x1a00, 9, slave_0_pdo_entries + 5}, /* Transmit PDO mapping 1 */
};

ec_sync_info_t slave_0_syncs[] = {
    {0, EC_DIR_OUTPUT, 0, NULL, EC_WD_DISABLE},
    {1, EC_DIR_INPUT, 0, NULL, EC_WD_DISABLE},
    {2, EC_DIR_OUTPUT, 1, slave_0_pdos + 0, EC_WD_ENABLE},
    {3, EC_DIR_INPUT, 1, slave_0_pdos + 1, EC_WD_DISABLE},
    {0xff}
};

/*****************************************************************************/

int MADHT1505BA1_master_init(void);
int MADHT1505BA1_slaves_init(MADHT1505BA1_object *object);
int MADHT1505BA1_master_activate(void);
int MADHT1505BA1_slaves_activate(MADHT1505BA1_object *object);
int MADHT1505BA1_master_deinit(void);
int MADHT1505BA1_slave_start(MADHT1505BA1_object *object);