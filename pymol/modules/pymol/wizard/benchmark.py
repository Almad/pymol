import threading
from pymol.wizard import Wizard
from pymol import cmd
import pymol
import types
import time

def bench_fn(action):
   time.sleep(0.5)
   cmd.do("_ wizard benchmark,%s"%action)

def report(name,value):
   print "PyMOL Benchmark: %30s = %10.5f"%(name,value)
   
class Benchmark(Wizard):

   def launch(self,name):
      return None

   def __init__(self,*arg):
      self.gl = 5.0
      self.short_cpu = 10.0
      self.long_cpu = 25.0
      self.message = []
      if len(arg):
         if hasattr(self,arg[0]):
            getattr(self,arg[0])()
            
   def reset(self):
      pass

   def run_all(self):
      self.run_gl()
      self.run_cpu()

   def run_cpu(self):
      self.surface_calculation()
      cmd.reinitialize()
      self.mesh_calculation()
      cmd.reinitialize()
      self.ray_tracing()
      cmd.reinitialize()

   def run_gl(self):
      cmd.reinitialize()
      self.updates()
      cmd.reinitialize()
      self.smooth_lines()
      cmd.reinitialize()
      self.jagged_lines()
      cmd.reinitialize()
      self.dots()
      cmd.reinitialize()
      self.sticks()
      cmd.reinitialize()
      self.surface()
      cmd.reinitialize()
      self.spheres()
      cmd.reinitialize()
      self.cartoon()
      cmd.reinitialize()

   def updates(self):
      cmd.fragment("methane")
      cmd.set("antialias",0)
      cnt = 0
      elapsed = 0.0
      cmd.refresh()
      cmd.meter_reset()
      start = time.time()
      while elapsed<self.gl: 
         cmd.turn("x",1)
         cmd.turn("y",1)
         cmd.refresh()
         cnt = cnt + 1
         elapsed = time.time()-start
      report('UPDATES',(cnt/elapsed)/100)

   def smooth_lines(self):
      cmd.load("test/dat/1tii.pdb")
      cmd.show("mesh")
      cmd.zoom(complete=1)
      elapsed = 0.0
      cnt = 0
      cmd.refresh()
      cmd.meter_reset()      
      start = time.time()
      while elapsed<self.gl: 
         cmd.turn("x",15)
         cmd.turn("y",15)
         cmd.refresh()
         cnt = cnt + 1
         elapsed = time.time()-start
      report('SMOOTH_LINES',cnt/elapsed)
      
   def jagged_lines(self):
      cmd.load("test/dat/1tii.pdb")
      cmd.show("mesh")
      cmd.set("line_smooth",0)
      cmd.zoom(complete=1)
      cnt = 0
      elapsed = 0.0
      cmd.refresh()
      cmd.meter_reset()      
      start = time.time()
      while elapsed<self.gl: 
         cmd.turn("x",15)
         cmd.turn("y",15)
         cmd.refresh()
         cnt = cnt + 1
         elapsed = time.time()-start
      report('JAGGED_LINES',cnt/elapsed)

   def dots(self):
      cmd.load("test/dat/1tii.pdb")
      cmd.hide()
      cmd.show("dots")
      cmd.zoom(complete=1)
      elapsed = 0.0
      cnt = 0
      cmd.refresh()
      cmd.meter_reset()      
      start = time.time()
      while elapsed<self.gl: 
         cmd.turn("x",15)
         cmd.turn("y",15)
         cmd.refresh()
         cnt = cnt + 1
         elapsed = time.time()-start
      report('DOTS',cnt/elapsed)

   def sticks(self):
      cmd.load("test/dat/1tii.pdb")
      cmd.hide()
      cmd.show("sticks")
      cmd.zoom(complete=1)
      cnt = 0
      elapsed = 0.0
      cmd.refresh()
      cmd.meter_reset()      
      start = time.time()
      while elapsed<self.gl: 
         cmd.turn("x",15)
         cmd.turn("y",15)
         cmd.refresh()
         cnt = cnt + 1
         elapsed = time.time()-start
      report('STICKS',cnt/elapsed)

   def surface(self):
      cmd.load("test/dat/1tii.pdb")
      cmd.hide()
      cmd.show("surface")
      cmd.zoom(complete=1)
      cnt = 0
      elapsed = 0.0
      cmd.refresh()
      cmd.meter_reset()      
      start = time.time()
      while elapsed<self.gl: 
         cmd.turn("x",15)
         cmd.turn("y",15)
         cmd.refresh()
         cnt = cnt + 1
         elapsed = time.time()-start
      report('SURFACE',cnt/elapsed)

   def spheres(self):
      cmd.load("test/dat/1tii.pdb")
      cmd.hide()
      cmd.show("spheres")
      cmd.zoom(complete=1)
      cnt = 0
      elapsed = 0.0
      cmd.refresh()
      cmd.meter_reset()      
      start = time.time()
      while elapsed<self.gl: 
         cmd.turn("x",15)
         cmd.turn("y",15)
         cmd.refresh()
         cnt = cnt + 1
         elapsed = time.time()-start
      report('SPHERES',cnt/elapsed)

   def cartoon(self):
      cmd.load("test/dat/1tii.pdb")
      cmd.hide()
      cmd.show("cartoon")
      cmd.spectrum("count",selection="name ca")
      cmd.zoom(complete=1)
      cnt = 0
      elapsed = 0.0
      cmd.refresh()
      cmd.meter_reset()      
      start = time.time()
      while elapsed<self.gl: 
         cmd.turn("x",15)
         cmd.turn("y",15)
         cmd.refresh()
         cnt = cnt + 1
         elapsed = time.time()-start
      report('CARTOON',cnt/elapsed)

   def surface_calculation(self):
      cmd.load("test/dat/il2.pdb")
      cmd.zoom(complete=1)
      cmd.hide()
      cmd.show("surface")
      cmd.clip("slab",0)
      cnt = 0
      elapsed = 0.0
      cmd.refresh()
      start = time.time()
      while (elapsed)<self.short_cpu: # how many times can we flush the framebuffer?
         cmd.rebuild()
         cmd.refresh()
         cnt = cnt + 1
         elapsed = time.time()-start
      report('SURFACE_CALCULATION',60*cnt/elapsed)

   def mesh_calculation(self):
      cmd.load("test/dat/il2.pdb")
      cmd.zoom(complete=1)
      cmd.hide()
      cmd.show("mesh")
      cmd.clip("slab",0)
      cnt = 0
      elapsed = 0.0
      cmd.refresh()
      start = time.time()
      while (elapsed)<self.short_cpu: # how many times can we flush the framebuffer?
         cmd.rebuild()
         cmd.refresh()
         cnt = cnt + 1
         elapsed = time.time()-start
      report('MESH_CALCULATION',60*cnt/elapsed)

   def ray_tracing(self):
      cmd.load("test/dat/1tii.pdb")
      cmd.zoom(complete=1)
      cmd.hide()
      cmd.show("spheres","11-20/")
      cmd.show("surface","21-30/")
      cmd.show("mesh","A//")
      cmd.show("sticks","41-50/")
      cmd.show("lines","51-60/")
      cmd.show("dots","61-70/")      
      cmd.show("cartoon","71-110/")
      cmd.turn('x',25)
      cmd.turn('y',25)
      cmd.set('hash_max','150')
      cnt = 0
      elapsed = 0.0
      cmd.refresh()
      start = time.time()
      while elapsed<self.long_cpu: # how many times can we flush the framebuffer?
         cmd.ray(200,200)
         cnt = cnt + 1
         elapsed = time.time()-start
      report('RAY_TRACING',60*cnt/elapsed)
      
   def get_prompt(self):
      self.prompt = self.message
      return self.prompt

   def delay_launch(self,action):
      cmd.reinitialize()
      cmd.viewport(640,480)
      cmd.feedback("disable","all","everything")
      cmd.feedback("enable","python","output")
      t = threading.Thread(target=bench_fn,args=(action,))
      t.setDaemon(1)
      t.start()
      
   def get_panel(self):
      return [
         [ 1, 'Benchmarks', '' ],
         [ 2, 'Run All', 'cmd.get_wizard().delay_launch("run_all")' ],
         [ 2, 'Run GL', 'cmd.get_wizard().delay_launch("run_gl")' ],
         [ 2, 'Run CPU', 'cmd.get_wizard().delay_launch("run_cpu")' ],                  
         [ 2, 'Updates', 'cmd.get_wizard().delay_launch("updates")'],
         [ 2, 'Smooth Lines', 'cmd.get_wizard().delay_launch("smooth_lines")'],
         [ 2, 'Dots', 'cmd.get_wizard().delay_launch("dots")'],         
         [ 2, 'Jagged Lines', 'cmd.get_wizard().delay_launch("jagged_lines")'],
         [ 2, 'Sticks', 'cmd.get_wizard().delay_launch("sticks")'],
         [ 2, 'Surface', 'cmd.get_wizard().delay_launch("surface")'],
         [ 2, 'Spheres', 'cmd.get_wizard().delay_launch("spheres")'],
         [ 2, 'Cartoon', 'cmd.get_wizard().delay_launch("cartoon")'],                           
         [ 2, 'Jagged Lines', 'cmd.get_wizard().delay_launch("jagged_lines")'],
         [ 2, 'Surface Calculation', 'cmd.get_wizard().delay_launch("surface_calculation")'],
         [ 2, 'Mesh Calculation', 'cmd.get_wizard().delay_launch("surface_calculation")'],
         [ 2, 'Ray Tracing', 'cmd.get_wizard().delay_launch("ray_tracing")'],         
         [ 2, 'End Demonstration', 'cmd.set_wizard()' ]
         ]
