var StlThumbnailer = require('node-stl-to-thumbnail');
var fs = require('fs');
var path = require('path');

var outputDirectory = path.join(__dirname, 'thumbnails'); // Here's the output directory, set to a 'output' folder in the current directory

// Ensure the output directory exists
if (!fs.existsSync(outputDirectory)){
    fs.mkdirSync(outputDirectory);
}

// Read all files from current working directory
fs.readdir(__dirname, function(err, files) {
    if (err) {
        console.error("Error reading directory:", err);
        return;
    }

    // Filter .stl files
    var stlFiles = files.filter(function(file) {
        return path.extname(file).toLowerCase() === ".stl";
    });

    // Loop through each .stl file and generate thumbnail
    stlFiles.forEach(function(stlFile) {
        new StlThumbnailer({
            filePath: path.join(__dirname, stlFile),
            requestThumbnails: [
                {
                    width: 250,
                    height: 250
                }
            ] 	
        })
        .then(function(thumbnails){
            // Determine the output file name by replacing .stl with .png
            var outputFileName = stlFile.replace('.stl', '.png');
            
            thumbnails[0].toBuffer(function(err, buf) {
                if (err) {
                    console.error("Error writing thumbnail:", err);
                    return;
                }
                
                fs.writeFileSync(path.join(outputDirectory, outputFileName), buf); // Write to the specified output directory
                console.log("Thumbnail generated for:", stlFile);
            });
        })
        .catch(function(error) {
            console.error("Error generating thumbnail for", stlFile, ":", error);
        });
    });
});
