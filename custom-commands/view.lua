-- Table mapping file extensions to the names of applications with
-- which to open files with the extensions.

apps = {
   ["txt"] = "gedit"
}

local selected = Nuc.source:selected()

if selected and selected:file_type() == NucEntry.TYPE_REG then
   local app = apps[selected:extension()]

   if app then
      Nuc.window:unpack_file(
         Nuc.source, selected,
         function(path)
            Nuc.open_with(app, path)
         end
      )
   end
end
