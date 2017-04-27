# Simple ruby script that constructs the readme file from various sources
# The primary purpose of this script is to automate the process of updating semantic versioning
# However, this can also be extended in the future to perform various tasks

require_relative '../Ruby/utils'

# TODO: Add in argument hooks to have more fine grain control over the generation
File.open(Dir.pwd + "/README.md", "w") do |readme|
    # Semantic versioning (TODO: Figure out a way to "save" this)
    major = 0       # Updated by hand
    minor = 1
    patch = 0
    readme.puts "speroc ver #{major}.#{minor}.#{patch} - The reference compiler for the spero language"


    # Information about files in the project (TODO: See if I can add in more stats)
    stats = SourceStats.new(["incl", "src", "main.cpp", "test.rb"])
    num_lines = stats.exts.reduce(0) do |acc, elem|
        acc + elem[1][:sloc]
    end

    readme.puts "\nProject Info:"
    readme.puts "\n    size: #{num_lines} sloc"
    readme.puts "    #{stats.exts['.h'][:files]} .h files, #{stats.exts['.cpp'][:files]} .cpp files, #{stats.exts['.rb'][:files]} .rb files"


    # TODO: Other stuff that I can possibly add in 


    # Move over the custom readme information into the public readme
    File.open(Dir.pwd + "/_readme.md", "r").each_line do |line|
        readme.puts line
    end
end