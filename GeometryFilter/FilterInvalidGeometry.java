import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;

import com.esri.core.geometry.GeometryException;
import com.esri.core.geometry.ogc.OGCGeometry;


public class FilterInvalidGeometry {

	public static void main(final String[] args) throws IOException {
		
		if(args.length != 2) {
			System.err.println("Usage: FilterInvalidGeometry <Input-File> <Output-File>");
			System.exit(-1);
		}
		
		final File inputFile = new File(args[0]);
		final File outputFile = new File(args[1]);
		
		if(! inputFile.exists()) {
			System.err.println("Unable to open: " + inputFile);
			System.exit(-1);
		}
		
		if(outputFile.exists()) {
			System.err.println("Output file already exists: " + outputFile);
			System.exit(-1);
		}
		
		final BufferedReader fileReader = new BufferedReader(new FileReader(inputFile));
		final BufferedWriter output = new BufferedWriter(new FileWriter(outputFile));
		
		int forwardedLines = 0;
		int rejectedLines = 0;
		
		String line = null;
		while((line = fileReader.readLine()) != null) {
			final String[] fields = line.split("\t");
			
			if(fields.length < 2) {
				System.err.println("Unable to parse: " + line + " splits " + fields.length);
			} else {
				try {
					final String formatedString = fields[0].replace("\"", "");
					final OGCGeometry geom = OGCGeometry.fromText(formatedString);
					geom.asBinary();
					
					output.write(line);
					output.write("\n");
					forwardedLines++;
				} catch(Exception e) {
					System.err.println("Exception: Unable to parse: " + fields[0]);
					rejectedLines++;
				}
			}
		}
		
		fileReader.close();
		output.close();
		
		System.out.println("Forwarded lines: " + forwardedLines + " / Rejected lines: " + rejectedLines);
	}
}
