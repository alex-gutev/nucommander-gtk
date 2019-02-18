-- Table mapping file extensions to the names of applications with
-- which to open files with the extensions.

apps = {
   ["txt"] = "gedit"
}

local selected = source_pane:selected()

if selected and selected:file_type() == NucEntry.TYPE_REG then
   local app = apps[selected:extension()]

   if app then
      window:unpack_file(
         source_pane, selected,
         function(path)
            Nuc.open_with(app, path)
         end
      )
   end
end
