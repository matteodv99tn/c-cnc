/**
 *  ____  _            _           _               
 * | __ )| | ___   ___| | __   ___| | __ _ ___ ___ 
 * |  _ \| |/ _ \ / __| |/ /  / __| |/ _` / __/ __|
 * | |_) | | (_) | (__|   <  | (__| | (_| \__ \__ \
 * |____/|_|\___/ \___|_|\_\  \___|_|\__,_|___/___/
 * 
 * @file block.h
 * 
 * Block class containing all the information associated
 * to a G-code line. It contains the information of the block type
 * as well as all the datas viable for the movement. 
 * In particular the entry point is set
 *                                                
 */

#ifndef BLOCK_H
#define BLOCK_H

#include "defines.h"
#include "point.h"
#include "machine.h"

/**
 *   _____                      
 *  |_   _|   _ _ __   ___  ___ 
 *    | || | | | '_ \ / _ \/ __|
 *    | || |_| | |_) |  __/\__ \
 *    |_| \__, | .__/ \___||___/
 *        |___/|_|              
 *
 * 
 * @struct block_t
 * @brief Structure containing all information of a G-code line
 * 
 * This structure contains all the information that might be useful
 * for following the path described by a single G-code line
 * 
 */
typedef struct block block_t;

/**
 * @brief Type of motion of the tool
 * 
 * Note that the enum type match perfectly the associated G-code line
 */
typedef enum {
  RAPID = 0,    //!< Rapid motion
  LINE,         //!< Straight line motion
  ARC_CW,       //!< Clockwise arc
  ARC_CCW,      //!< Counter-clocwise arc
  NO_MOTION     //!< No motion
} block_type_t;

/**
 *   _____                 _   _                 
 *  |  ___|   _ _ __   ___| |_(_) ___  _ __  ___ 
 *  | |_ | | | | '_ \ / __| __| |/ _ \| '_ \/ __|
 *  |  _|| |_| | | | | (__| |_| | (_) | | | \__ \
 *  |_|   \__,_|_| |_|\___|\__|_|\___/|_| |_|___/
 * 
 *  Main function implemented for the block_t struct
 * 
 */

/**
 * @brief Function that allocates memory for a G-code block type
 * 
 * @param line original G-line soource text
 * @param prev pointer to the previous G-code block (previous line)
 * @param cfg  configuration information of a machine
 * 
 * @return pointer to the newly created memory
 * 
 */
block_t *block_new(const char *line, block_t *prev, machine_t *cfg);

/**
 * @brief Destructor function of the G-code block type
 * 
*/
void block_free(block_t *b);

/**
 * @brief Prints general details of the block
 * 
 * @param b   block to print information of
 * @param out output stream for the information (if `NULL`: `stoud`)
 * 
 */
void block_print(block_t *b, FILE *out);

/**
 *      _    _                  _ _   _                   
 *     / \  | | __ _  ___  _ __(_) |_| |__  _ __ ___  ___ 
 *    / _ \ | |/ _` |/ _ \| '__| | __| '_ \| '_ ` _ \/ __|
 *   / ___ \| | (_| | (_) | |  | | |_| | | | | | | | \__ \
 *  /_/   \_\_|\__, |\___/|_|  |_|\__|_| |_|_| |_| |_|___/
 *             |___/                              
 * 
 *  Algorithms used to determine the tool position
 *  within each G-code block
 *         
 */

/**
 * @brief Parses the G-code line stored in the block_t struct
 * 
 * @param b block to parse 
 *  
 */
int block_parse(block_t *b);

/**
 * @brief local coordinate lambda of the motion
 * 
 * This function returns the value of the local coordinate
 * (lambda in the slides) along the movement trajectory 
 * depending on the desired time. In this case time is 
 * assumed to be a relative value starting from the start
 * of the block (so for t = 0 we have lambda = 0)
 * 
 * @param b    block from which to retrieve the information
 * @param time relative time on which lambda is defined
 * @param v    current velocity of the tip
 */
data_t block_lambda(const block_t *b, data_t time, data_t *v);

/**
 * @brief Determine the absolute position of the tip
 * associated to a certaing lambda
 * 
 * @param b      block from which extract the interpolation
 * @param lambda local coordinate of the block
 */
point_t *block_interpolate(block_t *b, data_t lambda);


/**
 *    ____      _   _                
 *   / ___| ___| |_| |_ ___ _ __ ___ 
 *  | |  _ / _ \ __| __/ _ \ '__/ __|
 *  | |_| |  __/ |_| ||  __/ |  \__ \
 *   \____|\___|\__|\__\___|_|  |___/
 *                                    
 */

/**
 * @brief  Returns the overall length of the trajectory of a G-code block
 * 
 * @param  b block from which extract the information
 * @return length of the trajectory
 */
data_t block_length(const block_t *b);

/**
 * @brief  Returns the total angle of an arc motion block
 * 
 * @param  b block from which extract the information
 * @return total angle of the arc motion
 */
data_t block_dtheta(const block_t *b);

/**
 * @brief  Returns the total time of execution of the block
 * 
 * @param  b block from which extract the information
 * @return overall time for executing the block
 */
data_t block_dt(const block_t *b);

/**
 * @brief  Returns the radius of an arc motion
 * 
 * @param  b block from which extract the information
 * @return radius of the arc
 */
data_t block_r(const block_t *b);

/**
 * @brief  Returns the type of the current block
 * 
 * @param  b block from which extract the information
 * @return block type
 */
block_type_t block_type(const block_t *b);

/**
 * @brief  Returns the pointer to the original G-code line
 * 
 * @param  b block from which extract the information
 * @return G-code line string
 */
char *block_line(const block_t *b);

/**
 * @brief  Returns the ID number of the G-code block
 * 
 * @param  b block from which extract the information
 * @return ID number of the block
 */
size_t block_n(const block_t *b);

/**
 * @brief  Returns the center point of the arc
 * 
 * @param  b block from which extract the information
 * @return center of the arc
 */
point_t *block_center(const block_t *b);

/**
 * @brief returns the next G-code block (as it is a linkedlist)
 * 
 * @param  b block from which extract the information
 * @return next G-code block
 */
block_t *block_next(const block_t *b);


#endif // BLOCK_H