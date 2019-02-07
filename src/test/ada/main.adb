--
-- \brief  Simple ada test program
-- \author Alexander Senier
-- \date   2019-01-03
--

with GNAT.IO;
with Except;
with Ada.Command_Line;

procedure Main is
   use GNAT.IO;
begin
   declare
   begin
      Put_Line ("Hello World!");
      Except.Do_Sth_2;
      Put_Line ("Bye World!");
      Except.Do_Something;
   exception
      when Except.Test_Exception => Put_Line ("Exception caught (outer)");
   end;

   Put_Line ("Program terminated");
   Ada.Command_Line.Set_Exit_Status (42);
end Main;
