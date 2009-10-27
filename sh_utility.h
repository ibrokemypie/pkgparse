/* Copyright (c) 2009 Sebastian Nowicki <sebnow@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef SH_UTILITY_H
#define SH_UTILITY_H

/** Parse a bash array into a string array
 *
 * The strings contained within the returned array will be normalised
 * and unquoted. The array, and each string is dynamically allocated and
 * should be freed by the user.
 *
 * @see sh_unquote()
 * @param string The string containing an array. It should be in the
 * format "(a b c)"
 * @return A NULL-terminated array of strings
 */
char **sh_array(char *string);

/** Remove quotes and unescape escaped quotes
 *
 * A string such as '"foo \"bar\""' will be unquoted to produce
 * 'foo "bar"'
 *
 * @param string The string to be unquoted
 * @return A pointer to a new string on success, otherwise a null pointer.
 */
char *sh_unquote(char *string);

#endif