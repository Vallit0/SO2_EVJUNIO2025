import './App.css'
import { BrowserRouter as Router, Route, Routes } from 'react-router-dom';
import MemoryChart from './pages/Memory'
import Navbar from './components/Navbar'
import Pagefault from './pages/Pagefault';
import SwapChart from './pages/Swap';
import MemoryPagesChart from './pages/MemoryPages';
import MemoryProcesses from './pages/MemoryProcesses';

function App() {
  return (
    <div className='bg-white min-h-screen h-full'>
      <Router>
        <Navbar />
        <Routes>
          <Route path="/memory" element={<MemoryChart />} />
          <Route path="/swap" element={<SwapChart/>} />
          <Route path="/pagefaults" element={<Pagefault/>} />
          <Route path="/mpages" element={<MemoryPagesChart/>} />
          <Route path="/mprocesses" element={<MemoryProcesses/>} />
        </Routes>
      </Router>
    </div>
  )
}

export default App
