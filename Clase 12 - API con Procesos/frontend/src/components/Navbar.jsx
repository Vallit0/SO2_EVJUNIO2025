import React from 'react';
import { Link } from 'react-router-dom';

const Navbar = () => {
    return (
        <nav className="bg-white shadow">
            <div className="flex p-6 mx-auto text-gray-600 capitalize">
                <Link
                    to="/memory"
                    href="#"
                    className="border-b-2 border-transparent hover:text-gray-800 transition-colors duration-300 transform  hover:border-blue-500 mx-1.5 sm:mx-6"
                >
                    Memory
                </Link>
                <Link
                    to="/swap"
                    href="#"
                    className="border-b-2 border-transparent hover:text-gray-800 transition-colors duration-300 transform  hover:border-blue-500 mx-1.5 sm:mx-6"
                >
                    Swap
                </Link>
                <Link
                    to="/pagefaults"
                    href="#"
                    className="border-b-2 border-transparent hover:text-gray-800 transition-colors duration-300 transform  hover:border-blue-500 mx-1.5 sm:mx-6"
                >
                    Page Faults
                </Link>
                <Link
                    to="/mpages"
                    href="#"
                    className="border-b-2 border-transparent hover:text-gray-800 transition-colors duration-300 transform  hover:border-blue-500 mx-1.5 sm:mx-6"
                >
                    Memory Pages
                </Link>
                <Link
                    to="/mprocesses"
                    href="#"
                    className="border-b-2 border-transparent hover:text-gray-800 transition-colors duration-300 transform  hover:border-blue-500 mx-1.5 sm:mx-6"
                >
                    Memory Processes
                </Link>
            </div>
        </nav>
    );
}

export default Navbar;
