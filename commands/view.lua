-- Table mapping file extensions to the names of applications with
-- which to open files with the extensions.

-- Dictionary associating extensions to applications.

-- Each key is a file extension and the corresponding value is the
-- name of the application to open, for files with that extension.

apps = {
   ["txt"] = "gedit"
}

-- Default Application
local default = "gedit"

-- Command Body

local selected = Nuc.source:selected()

if selected and selected:file_type() == NucEntry.TYPE_REG then
   local app = apps[string.lower(selected:extension())] or default

   Nuc.window:unpack_file(
      Nuc.source, selected,
      function(path)
         Nuc.open_with(app, path)
      end
   )
end
