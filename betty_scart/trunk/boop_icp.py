#!/usr/bin/env python

# based on http://blog.theroyweb.com/embedding-a-binary-file-as-an-array-in-firmware

# Convert binary file to a hex encoded array for inclusion in C projects

import os
import sys
import struct


class BinToArray:
	def __init__(self):
		pass

	def ConvertFileToArray(self, strInFile, strOutFile, ignoreBytes):
		""" Reads binary file at location strInFile and writes out a C array of hex values
			Parameters -
				strInFile - Path and filename of binary file to convert
				strOutFile - Path and filename of output. Suggested extension is .c or .cpp
				ignoreBytes - Number of bytes to ignore at the beginning of binary file. Helps
					strip out file headers and only encode the payload/data.
		"""

		strVarType  = "const unsigned char"
		strVarName  = "scart_image"
		strValSpace = ""
		ValsPerLine = 15
		ValCount	= 0
		ValsTotal   = 0x1C00 #7168

		# Open input file
		try:
			fileIn = open(strInFile, 'rb')
		except IOError, err:
			logging.debug("Could not open input file %s" % (strInFile))
			return
		# end try

		# Open output file
		try:
			fileOut = open(strOutFile, 'w')
		except IOError, err:
			logging.debug("Could not open output file %s" % (strOutFile))
			return
		# end try

		# Start array definition preamble
		inFileName = os.path.basename(strInFile)
		fileOut.write("// Array representation of binary file %s\n\n\n" % (inFileName))
		fileOut.write("%s %s[] = \n{\n" % (strVarType, strVarName) )

		# Convert and write array into C file

		# ignore header
		fileIn.seek(ignoreBytes)

		# output hex bytes linewise
		while True:
			if ValCount + ValsPerLine > ValsTotal:
				ValsPerLine = ValsTotal - ValCount
			if ValsPerLine == 0:
				break
			ValCount += ValsPerLine

			# read data
			bufChunk = fileIn.read(ValsPerLine)

			# write line of hex bytes
			fileOut.write("\t")
			for byteVal in bufChunk:
				fileOut.write("0x%02x,%s" % (ord(byteVal), strValSpace))
			fileOut.write("\n")


		# Close files
		fileIn.close()

		fileOut.write("};\n")
		fileOut.close()

		sys.stderr.write("Converted %d values.\n\n" % (ValCount))

if __name__ == "__main__":
	converter = BinToArray()
	converter.ConvertFileToArray("betty_scart.bin", "../../boop/trunk/tools/scart_image_data.h", 0, )
