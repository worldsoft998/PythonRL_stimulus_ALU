// dpi/dpi_imports.sv
import "DPI-C" function int dpi_socket_connect(input string host, input int port);
import "DPI-C" function int dpi_socket_close();
import "DPI-C" function int dpi_socket_send_json(input string json_str, input int len);
import "DPI-C" function int dpi_socket_recv_json(output string buf, input int buf_len);

// Batch functions: send JSON array of observations, receive JSON array of actions and parse into C arrays
import "DPI-C" function int dpi_send_obs_batch(input string json_array, input int len);
import "DPI-C" function int dpi_recv_actions_parse(); // receives next JSON reply and parse
import "DPI-C" function int dpi_get_action_count();
import "DPI-C" function int dpi_get_action_op(input int idx); // returns op code 0..4
import "DPI-C" function int dpi_get_action_a(input int idx);
import "DPI-C" function int dpi_get_action_b(input int idx);
