import React, { useState, useEffect } from 'react';
import Chart from "chart.js/auto";
import { Bar } from 'react-chartjs-2';

const MemoryProcesses = () => {
    const API_URL = import.meta.env.VITE_API_URL
    const ENDPOINT_MPROCESSES = import.meta.env.VITE_ENDPOINT_MPROCESSES

    const [processesData, setProcessesData] = useState([]);

    const updateProcessesData = async () => {
        const requestOptions = {
            method: 'GET',
        }
        const response = await fetch(API_URL + ENDPOINT_MPROCESSES, requestOptions);
        const data = await response.json();
        setProcessesData(data);
    }

    useEffect(() => {
        // Esta función se ejecutará cada 5000 milisegundos (5 segundos)
        const intervalId = setInterval(() => {
            updateProcessesData();
        }, 2000);

        // Limpieza del intervalo cuando el componente se desmonta o actualiza
        return () => clearInterval(intervalId);
    }, []);

    const data = {
        labels: processesData.map((d) => d.pid),
        datasets: [{
            label: 'Memoria utilizada',
            data: processesData.map((d) => d.memory),
            borderColor: 'rgba(192, 75, 75, 1)',
        }]
    }
    const options = {
        indexAxis: 'y',
        // Elements options apply to all of the options unless overridden in a dataset
        // In this case, we are setting the border of each horizontal bar to be 2px wide
        plugins: {
            legend: {
                position: 'right',
            },
        },
    }

    return (
        <div className='container mx-auto py-5 h-full'>
            <h1 className="text-center text-5xl my-8">Procesos y uso de memoria</h1>
            <div className="flex flex-row w-full justify-between">
                {/* Tabla a la izquierda */}
                <div className="w-1/5 p-5">
                    <h2 className="text-center text-3xl mb-2">Tabla de Procesos</h2>
                    <br></br>
                    <table className="table-auto border-collapse border border-gray-300 w-full">
                        <thead>
                            <tr>
                                <th className="bg-gray-100 border border-gray-300 px-4 py-2">PID</th>
                                <th className="bg-gray-100 border border-gray-300 px-4 py-2">Memory (Bytes)</th>
                            </tr>
                        </thead>
                        <tbody>
                            {processesData.map((item, index) => (
                                <tr key={index} className="hover:bg-gray-100">
                                    <td className="bg-gray-50 border border-gray-300 px-4 py-2">{item.pid}</td>
                                    <td className="bg-white border border-gray-300 px-4 py-2">{item.memory}</td>
                                </tr>
                            ))}
                        </tbody>
                    </table>
                </div>
    
                {/* Gráfica Bar a la derecha */}
                <div className="w-4/5 p-5">
                    <h2 className="text-center text-3xl mb-2">Gráfica de procesos</h2>
                    <div className="py-5 px-5">
                        <Bar data={data} options={options} height={"25rem"} width={"100%"} />
                    </div>
                </div>
            </div>
        </div>
    )
}

export default MemoryProcesses;